#pragma once

#include "userver/formats/json/value_builder.hpp"
#include "userver/server/handlers/exceptions.hpp"

namespace realworld::errors {

userver::formats::json::Value MakeError(std::string_view field,
                                        std::string_view msg);

class ErrorBuilder {
 public:
  static constexpr bool kIsExternalBodyFormatted{true};

  ErrorBuilder(std::string_view field, std::string_view msg);
  std::string GetExternalBody() const;

 private:
  std::string json_error_body_;
};

// at the time of writing this code,
// there is no HandlerErrorCode for 422 http status
class ValidationError
    : public userver::server::handlers::ExceptionWithCode<
          userver::server::handlers::HandlerErrorCode::kClientError> {
 public:
  using BaseType::BaseType;

  userver::formats::json::Value ToJson() const;
};

class ForbiddenError
    : public userver::server::handlers::ExceptionWithCode<
          userver::server::handlers::HandlerErrorCode::kForbidden> {
 public:
  using BaseType::BaseType;
};

}  // namespace realworld::errors