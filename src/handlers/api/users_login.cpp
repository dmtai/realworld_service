#include "users_login.hpp"
#include <userver/storages/secdist/component.hpp>
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/jwt.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/auth.hpp"
#include "dto/user.hpp"
#include "models/user.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::users_login::post {

namespace {

dto::LoginRequest ParseRequest(const userver::formats::json::Value& data) {
  dto::LoginRequest login_request;
  login_request.email_ = utils::ValidateEmail(data, "email");
  login_request.password_ = utils::CheckSize(data, "password", 5, 100);
  return login_request;
}

}  // namespace

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cluster_(context
                   .FindComponent<userver::components::Postgres>(
                       "realworld-database")
                   .GetCluster()),
      jwt_manager_(context.FindComponent<userver::components::Secdist>()
                       .Get()
                       .Get<jwt::JWTConfig>()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  dto::LoginRequest login_request;
  try {
    login_request = ParseRequest(request_json["user"]);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                        db::sql::kGetUserByEmail.data(), login_request.email_);
  if (res.IsEmpty()) {
    throw errors::ForbiddenError{errors::ErrorBuilder{"email", "invalid"}};
  }
  const auto user = res.AsSingleRow<models::User>();
  if (!BCrypt::validatePassword(login_request.password_, user.hash_)) {
    throw errors::ForbiddenError{errors::ErrorBuilder{"password", "invalid"}};
  }
  userver::formats::json::ValueBuilder builder;
  builder["user"] = dto::User{user.email_, jwt_manager_.GenerateToken(user.id_),
                              user.username_, user.bio_, user.image_};
  return builder.ExtractValue();
}

}  // namespace realworld::handlers::api::users_login::post