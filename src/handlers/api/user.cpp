#include "user.hpp"
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/profile.hpp"
#include "dto/user.hpp"
#include "models/profile.hpp"
#include "models/user.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::user {

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
  const auto& user_auth_data =
      request_context.GetData<auth::UserAuthData>("user_auth_data");
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        db::sql::kGetUserById.data(), user_auth_data.id_);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
  const auto user = res.AsSingleRow<models::User>();
  userver::formats::json::ValueBuilder builder;
  builder["user"] = dto::User{user.email_, user_auth_data.token_.get_token(),
                              user.username_, user.bio_, user.image_};
  return builder.ExtractValue();
}

}  // namespace get

namespace put {

namespace {

dto::UpdateUserRequest ParseRequest(const userver::formats::json::Value& data) {
  dto::UpdateUserRequest update_req;
  if (update_req.email_ = data["email"].As<std::optional<std::string>>();
      update_req.email_) {
    utils::ValidateEmail(data, "email");
  }
  if (update_req.username_ = data["username"].As<std::optional<std::string>>();
      update_req.username_) {
    utils::CheckSize(*update_req.username_, "username", 2, 20);
  }
  if (update_req.password_ = data["password"].As<std::optional<std::string>>();
      update_req.password_) {
    utils::CheckSize(*update_req.password_, "password", 5, 100);
  }
  if (update_req.bio_ = data["bio"].As<std::optional<std::string>>();
      update_req.bio_) {
    utils::CheckSize(*update_req.bio_, "bio", 3, 65535);
  }
  if (update_req.image_ = data["image"].As<std::optional<std::string>>();
      update_req.image_) {
    utils::CheckSize(*update_req.image_, "image", 3, 255);
  }
  if (!update_req.email_ && !update_req.username_ && !update_req.password_ &&
      !update_req.bio_ && !update_req.image_) {
    throw errors::ValidationError{
        errors::ErrorBuilder{"user", "cannot be empty"}};
  }
  return update_req;
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
  dto::UpdateUserRequest update_user_request;
  try {
    update_user_request = ParseRequest(request_json["user"]);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }
  const auto& user_auth_data =
      request_context.GetData<auth::UserAuthData>("user_auth_data");
  const auto password_hash =
      update_user_request.password_
          ? std::make_optional<std::string>(
                BCrypt::generateHash(*update_user_request.password_))
          : std::nullopt;

  const auto res = cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      db::sql::kUpdateUserById.data(), user_auth_data.id_,
      update_user_request.username_, update_user_request.email_, password_hash,
      update_user_request.bio_, update_user_request.image_);
  if (res.IsEmpty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }

  const auto user = res.AsSingleRow<models::User>();
  userver::formats::json::ValueBuilder builder;
  builder["user"] = dto::User{user.email_, user_auth_data.token_.get_token(),
                              user.username_, user.bio_, user.image_};
  return builder.ExtractValue();
}

}  // namespace put

}  // namespace realworld::handlers::api::user