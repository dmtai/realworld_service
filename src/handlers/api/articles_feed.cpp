#include "articles_feed.hpp"
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

namespace realworld::handlers::api::articles_feed::get {

namespace {

dto::FeedRequest ParseRequest(
    const userver::server::http::HttpRequest& request) {
  dto::FeedRequest filters;
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
  const auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      db::sql::kGetFeed.data(),
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_,
      filters.limit_, filters.offset_);
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

}  // namespace realworld::handlers::api::articles_feed::get