#include "articles_slug_comments.hpp"
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/article.hpp"
#include "dto/comment.hpp"
#include "models/article.hpp"
#include "models/comment.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::articles_slug_comments {

namespace get {

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cluster_(context
                   .FindComponent<userver::components::Postgres>(
                       "realworld-database")
                   .GetCluster()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext& request_context) const {
  const auto& slug = request.GetPathArg("slug");
  const auto* user_auth_data =
      request_context.GetDataOptional<auth::UserAuthData>("user_auth_data");
  const auto user_id =
      user_auth_data ? std::make_optional<std::int32_t>(user_auth_data->id_)
                     : std::nullopt;
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        db::sql::kGetCommentsFromArticle.data(), slug, user_id);
  if (res.IsEmpty()) {
    return {};
  }
  const auto comments = res.AsSetOf<models::Comment>();
  userver::formats::json::ValueBuilder builder;
  std::for_each(comments.begin(), comments.end(),
                [&builder](const auto& comment) {
                  builder["comments"].PushBack(dto::Comment::Parse(comment));
                });
  return builder.ExtractValue();
}

}  // namespace get

namespace post {

namespace {

dto::NewCommentRequest ParseRequest(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& request_context) {
  dto::NewCommentRequest comment;
  comment.slug_ = request.GetPathArg("slug");
  comment.user_id_ =
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_;
  comment.body_ = utils::CheckSize(request_json, "body", 4, 16384);
  return comment;
}

}  // namespace

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cluster_(context
                   .FindComponent<userver::components::Postgres>(
                       "realworld-database")
                   .GetCluster()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& request_context) const {
  dto::NewCommentRequest new_comment_request;
  try {
    new_comment_request =
        ParseRequest(request, request_json["comment"], request_context);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }
  auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      db::sql::kAddNewComment.data(), new_comment_request.slug_,
      new_comment_request.body_, new_comment_request.user_id_);

  const auto comment_id = res.AsSingleRow<std::int32_t>();
  res = cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                          db::sql::kGetComment.data(), comment_id,
                          new_comment_request.user_id_);

  userver::formats::json::ValueBuilder builder;
  builder["comment"] = dto::Comment::Parse(res.AsSingleRow<models::Comment>());
  return builder.ExtractValue();
}

}  // namespace post

namespace del {

namespace {

dto::DeleteCommentRequest ParseRequest(
    const userver::server::http::HttpRequest& request) {
  dto::DeleteCommentRequest data;
  data.slug_ = request.GetPathArg("slug");
  data.id_ = boost::lexical_cast<std::int32_t>(request.GetPathArg("id"));
  return data;
}

}  // namespace

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cluster_(context
                   .FindComponent<userver::components::Postgres>(
                       "realworld-database")
                   .GetCluster()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext& request_context) const {
  const auto del_comment_request = ParseRequest(request);
  const auto user_id =
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_;
  const auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      db::sql::kIsCommentExist.data(), del_comment_request.id_,
      del_comment_request.slug_, user_id);
  if (!res.AsSingleRow<bool>()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                    db::sql::kDeleteComment.data(), del_comment_request.id_,
                    del_comment_request.slug_, user_id);
  return {};
}

}  // namespace del

}  // namespace realworld::handlers::api::articles_slug_comments