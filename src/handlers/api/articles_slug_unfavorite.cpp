#include "articles_slug_unfavorite.hpp"
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/article.hpp"
#include "models/article.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::articles_slug_unfavorite::del {

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
  auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetArticleIdBySlug.data(), slug);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  const auto user_id =
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_;
  cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                    db::sql::kUnfavoriteArticle.data(), slug, user_id);
  const auto article_id = res.AsSingleRow<std::int32_t>();
  res = cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                          db::sql::kGetArticleWithAuthorProfile.data(),
                          article_id, user_id);
  userver::formats::json::ValueBuilder builder;
  builder["article"] =
      dto::Article::Parse(res.AsSingleRow<models::ArticleWithAuthorProfile>());
  return builder.ExtractValue();
}

}  // namespace realworld::handlers::api::articles_slug_unfavorite::del