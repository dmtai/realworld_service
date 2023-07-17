#include "users.hpp"
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
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::users::post {

namespace {

dto::RegistrationRequest ParseRequest(
    const userver::formats::json::Value& data) {
  dto::RegistrationRequest reg_request;
  reg_request.username_ = utils::CheckSize(data, "username", 2, 20);
  reg_request.password_ = utils::CheckSize(data, "password", 5, 100);
  reg_request.email_ = utils::ValidateEmail(data, "email");
  return reg_request;
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
  dto::RegistrationRequest reg_request;
  try {
    reg_request = ParseRequest(request_json["user"]);
  } catch (const errors::ValidationError& ex) {
    request.SetResponseStatus(
        userver::server::http::HttpStatus::kUnprocessableEntity);
    return ex.ToJson();
  }

  std::int32_t user_id{};
  try {
    const auto password_hash = BCrypt::generateHash(reg_request.password_);
    const auto res =
        cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                          db::sql::kAddNewUser.data(), reg_request.username_,
                          reg_request.email_, password_hash);
    user_id = res.AsSingleRow<std::int32_t>();
  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    const auto constraint = ex.GetServerMessage().GetConstraint();
    std::optional<std::string> name;
    if (constraint == "uniq_username") {
      name = "username";
    } else if (constraint == "uniq_email") {
      name = "email";
    }
    if (name) {
      request.SetResponseStatus(
          userver::server::http::HttpStatus::kUnprocessableEntity);
      return errors::MakeError(*name, "has already been taken");
    }
    throw;
  }

  userver::formats::json::ValueBuilder builder;
  builder["user"] =
      dto::User{reg_request.email_, jwt_manager_.GenerateToken(user_id),
                reg_request.username_};
  return builder.ExtractValue();
}

}  // namespace realworld::handlers::api::users::post