#pragma once

#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include "common/jwt.hpp"

namespace realworld::handlers::auth {

class AuthChecker final
    : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using AuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  AuthChecker(const jwt::JWTConfig& config, bool is_optional_auth = false)
      : jwt_manager_{config}, is_optional_auth_{is_optional_auth} {}

  [[nodiscard]] AuthCheckResult CheckAuth(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& request_context) const override;

  [[nodiscard]] bool SupportsUserAuth() const noexcept override { return true; }

 private:
  const jwt::JWTManager jwt_manager_;
  const bool is_optional_auth_;
};

class CheckerFactory final
    : public userver::server::handlers::auth::AuthCheckerFactoryBase {
 public:
  userver::server::handlers::auth::AuthCheckerBasePtr operator()(
      const userver::components::ComponentContext& context,
      const userver::server::handlers::auth::HandlerAuthConfig& auth_config,
      const userver::server::handlers::auth::AuthCheckerSettings&)
      const override;
};

}  // namespace realworld::handlers::auth