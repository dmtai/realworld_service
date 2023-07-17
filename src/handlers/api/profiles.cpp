#include "profiles.hpp"
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/profile.hpp"
#include "models/profile.hpp"
#include "models/user.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::profiles {

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
  const auto& username = request.GetPathArg("username");
  const auto* user_auth_data =
      request_context.GetDataOptional<auth::UserAuthData>("user_auth_data");
  const auto user_id =
      user_auth_data ? std::make_optional<std::int32_t>(user_auth_data->id_)
                     : std::nullopt;
  const auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      db::sql::kGetProfileByUsername.data(), username, user_id);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  const auto profile = res.AsSingleRow<models::Profile>();
  userver::formats::json::ValueBuilder builder;
  builder["profile"] = dto::Profile{profile.username_, profile.bio_,
                                    profile.image_, profile.following_};
  return builder.ExtractValue();
}

}  // namespace get

namespace post {

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
  const auto& username = request.GetPathArg("username");
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetUserByUsername.data(), username);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  const auto user = res.AsSingleRow<models::User>();
  cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      db::sql::kFollow.data(),
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_,
      user.id_);
  userver::formats::json::ValueBuilder builder;
  builder["profile"] =
      dto::Profile{user.username_, user.bio_, user.image_, true};
  return builder.ExtractValue();
}

}  // namespace post

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
  const auto& username = request.GetPathArg("username");
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetUserByUsername.data(), username);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  const auto user = res.AsSingleRow<models::User>();
  cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      db::sql::kUnfollow.data(),
      request_context.GetData<auth::UserAuthData>("user_auth_data").id_,
      user.id_);
  userver::formats::json::ValueBuilder builder;
  builder["profile"] =
      dto::Profile{user.username_, user.bio_, user.image_, false};
  return builder.ExtractValue();
}

}  // namespace del

}  // namespace realworld::handlers::api::profiles