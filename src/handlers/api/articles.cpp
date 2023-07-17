#include "articles.hpp"
#include <boost/algorithm/string.hpp>
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "dto/article.hpp"
#include "models/article.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::articles {

namespace get {

namespace {

dto::ArticlesListRequest ParseRequest(
    const userver::server::http::HttpRequest& request) {
  dto::ArticlesListRequest filters;
  if (request.HasArg("tag")) {
    filters.tag_ = request.GetArg("tag");
  }
  if (request.HasArg("favorited")) {
    filters.favorited_ = request.GetArg("favorited");
  }
  if (request.HasArg("author")) {
    filters.author_ = request.GetArg("author");
  }
  if (request.HasArg("limit")) {
    filters.limit_ = boost::lexical_cast<std::int32_t>(request.GetArg("limit"));
  }
  if (request.HasArg("offset")) {
    filters.offset_ =
        boost::lexical_cast<std::int32_t>(request.GetArg("offset"));
  }
  return filters;
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
  const auto filters = ParseRequest(request);
  const auto* user_auth_data =
      request_context.GetDataOptional<auth::UserAuthData>("user_auth_data");
  const auto user_id =
      user_auth_data ? std::make_optional<std::int32_t>(user_auth_data->id_)
                     : std::nullopt;
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        db::sql::kGetArticlesWithAuthorProfile.data(),
                        filters.tag_, filters.author_, filters.favorited_,
                        user_id, filters.limit_, filters.offset_);
  const auto list_articles = res.AsSetOf<models::ArticleWithAuthorProfile>();
  userver::formats::json::ValueBuilder builder;
  builder["articles"] = userver::formats::common::Type::kArray;
  std::for_each(list_articles.begin(), list_articles.end(),
                [&builder](const auto& article) {
                  builder["articles"].PushBack(dto::Article::Parse(article));
                });
  builder["articlesCount"] = list_articles.Size();
  return builder.ExtractValue();
}

}  // namespace get

namespace post {

namespace {

dto::NewArticleRequest ParseRequest(const userver::formats::json::Value& data) {
  dto::NewArticleRequest new_article_request;
  new_article_request.title_ = utils::CheckSize(data, "title", 3, 256);
  new_article_request.description_ =
      utils::CheckSize(data, "description", 5, 8192);
  new_article_request.body_ = utils::CheckSize(data, "body", 5, 65535);
  new_article_request.tag_list_ =
      data["tagList"].As<std::optional<std::vector<std::string>>>();
  if (new_article_request.tag_list_) {
    std::for_each(new_article_request.tag_list_->begin(),
                  new_article_request.tag_list_->end(), [](auto& tag) {
                    utils::CheckSize(tag, "tagList", 2, 256);
                    boost::algorithm::to_lower(tag);
                  });
  }
  return new_article_request;
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
  dto::NewArticleRequest new_article_request;
  try {
    new_article_request = ParseRequest(request_json["article"]);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }
  const auto& user_auth_data =
      request_context.GetData<auth::UserAuthData>("user_auth_data");

  std::int32_t article_id{};
  try {
    const auto slug = slug::Slugify(new_article_request.title_);
    const auto res = cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        db::sql::kAddNewArticle.data(), new_article_request.title_, slug,
        new_article_request.description_, new_article_request.body_,
        user_auth_data.id_, new_article_request.tag_list_);

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

}  // namespace post

}  // namespace realworld::handlers::api::articles