#include "articles_slug.hpp"
#include <boost/algorithm/string.hpp>
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "dto/article.hpp"
#include "models/article.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::articles_slug {

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
  const auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      db::sql::kGetArticleWithAuthorProfileBySlug.data(), slug, user_id);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  userver::formats::json::ValueBuilder builder;
  builder["article"] =
      dto::Article::Parse(res.AsSingleRow<models::ArticleWithAuthorProfile>());
  return builder.ExtractValue();
}

}  // namespace get

namespace put {

namespace {

dto::UpdateArticleRequest ParseRequest(
    const userver::formats::json::Value& data,
    const userver::server::http::HttpRequest& request) {
  dto::UpdateArticleRequest article;
  article.slug_ = request.GetPathArg("slug");
  if (article.title_ = data["title"].As<std::optional<std::string>>();
      article.title_) {
    utils::CheckSize(*article.title_, "title", 3, 255);
  }
  if (article.description_ =
          data["description"].As<std::optional<std::string>>();
      article.description_) {
    utils::CheckSize(*article.description_, "description", 5, 8192);
  }
  if (article.body_ = data["body"].As<std::optional<std::string>>();
      article.body_) {
    utils::CheckSize(*article.body_, "body", 5, 65535);
  }
  article.tag_list_ =
      data["tagList"].As<std::optional<std::vector<std::string>>>();
  if (article.tag_list_) {
    std::for_each(article.tag_list_->begin(), article.tag_list_->end(),
                  [](auto& tag) {
                    utils::CheckSize(tag, "tagList", 2, 256);
                    boost::algorithm::to_lower(tag);
                  });
  }
  if (!article.title_ && !article.description_ && !article.body_ &&
      !article.tag_list_) {
    throw errors::ValidationError{
        errors::ErrorBuilder{"article", "cannot be empty"}};
  }
  return article;
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
  dto::UpdateArticleRequest update_article_request;
  try {
    update_article_request = ParseRequest(request_json["article"], request);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }
  const auto user_auth_data =
      request_context.GetData<auth::UserAuthData>("user_auth_data");

  std::int32_t article_id{};
  try {
    const auto new_slug = update_article_request.title_
                              ? std::make_optional<std::string>(slug::Slugify(
                                    *update_article_request.title_))
                              : std::nullopt;
    const auto res = cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        db::sql::kUpdateArticleBySlug.data(), update_article_request.slug_,
        user_auth_data.id_, update_article_request.title_, new_slug,
        update_article_request.description_, update_article_request.body_);
    if (res.IsEmpty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }
    article_id = res.AsSingleRow<std::int32_t>();
  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    const auto constraint = ex.GetServerMessage().GetConstraint();
    if (constraint == "uniq_slug") {
      request.SetResponseStatus(
          userver::server::http::HttpStatus::kUnprocessableEntity);
      return errors::MakeError("slug", "has already been taken");
    }
    throw;
  }
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetArticleWithAuthorProfile.data(),
                        article_id, user_auth_data.id_);

  userver::formats::json::ValueBuilder builder;
  builder["article"] =
      dto::Article::Parse(res.AsSingleRow<models::ArticleWithAuthorProfile>());
  return builder.ExtractValue();
}

}  // namespace put

namespace del {

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
  const auto user_id =
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_;
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetArticleIdBySlug.data(), slug);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                    db::sql::kDeleteArticleBySlug.data(), slug, user_id);
  return {};
}

}  // namespace del

}  // namespace realworld::handlers::api::articles_slug