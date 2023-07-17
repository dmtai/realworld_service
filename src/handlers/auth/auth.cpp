#include "auth.hpp"
#include <algorithm>
#include <memory>
#include <userver/storages/secdist/component.hpp>
#include "common/auth.hpp"

namespace realworld::handlers::auth {

AuthChecker::AuthCheckResult AuthChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto& auth_value = request.GetHeader("Authorization");
  if (auth_value.empty()) {
    if (is_optional_auth_) {
      return {};
    }
    return AuthCheckResult{
        AuthCheckResult::Status::kTokenNotFound,
        {},
        "Missing authorization token",
        userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  const auto token_pos = auth_value.find(' ');
  if (token_pos == std::string::npos ||
      std::string_view{auth_value.data(), token_pos} != "Token") {
    return AuthCheckResult{
        AuthCheckResult::Status::kTokenNotFound,
        {},
        "Missing authorization token",
        userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  const std::string token{auth_value.data() + token_pos + 1};
  std::optional<jwt::DecodedToken> decoded_token;
  try {
    decoded_token = ::jwt::decode(token);
    jwt_manager_.VerifyToken(*decoded_token);
  } catch (const std::exception&) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInvalidToken,
        {},
        "Invalid token",
        userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  const auto id = static_cast<std::int32_t>(
      (*decoded_token).get_payload_claim("id").as_integer());
  const realworld::auth::UserAuthData user_auth_data{id, *decoded_token};
  request_context.SetData("user_auth_data", user_auth_data);
  return {};
}

userver::server::handlers::auth::AuthCheckerBasePtr CheckerFactory::operator()(
    const userver::components::ComponentContext& context,
    const userver::server::handlers::auth::HandlerAuthConfig& config,
    const userver::server::handlers::auth::AuthCheckerSettings&) const {
  const auto jwt_config = context.FindComponent<userver::components::Secdist>()
                              .Get()
                              .Get<jwt::JWTConfig>();
  const auto is_optional_auth =
      config.HasMember("optional") ? config["optional"].As<bool>() : false;
  const auto res =
      std::make_shared<AuthChecker>(std::move(jwt_config), is_optional_auth);
  return res;
}

}  // namespace realworld::handlers::auth