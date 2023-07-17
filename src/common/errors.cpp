#include "errors.hpp"

namespace realworld::errors {

userver::formats::json::Value MakeError(std::string_view field,
                                        std::string_view msg) {
  userver::formats::json::ValueBuilder error;
  error["errors"][std::string{field}].PushBack(msg);
  return error.ExtractValue();
}

ErrorBuilder::ErrorBuilder(std::string_view field, std::string_view msg) {
  json_error_body_ = userver::formats::json::ToString(MakeError(field, msg));
}

std::string ErrorBuilder::GetExternalBody() const { return json_error_body_; }

userver::formats::json::Value ValidationError::ToJson() const {
  return userver::formats::json::FromString(GetExternalErrorBody());
}

}  // namespace realworld::errors