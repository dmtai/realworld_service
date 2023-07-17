#include "utils.hpp"
#include <regex>
#include "errors.hpp"
#include "fmt/core.h"
#include "unicode/translit.h"

namespace realworld::utils {

const std::string ValidateEmail(const userver::formats::json::Value& data,
                                std::string_view name) {
  if (!data.HasMember(name)) {
    throw errors::ValidationError{errors::ErrorBuilder{name, "required"}};
  }
  const auto& email = data[name].As<std::string>();
  if (email.empty()) {
    throw errors::ValidationError{
        errors::ErrorBuilder{name, "cannot be empty"}};
  }
  const std::regex pattern{"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"};
  if (std::regex_match(email, pattern)) {
    return email;
  }
  throw errors::ValidationError{errors::ErrorBuilder{name, "invalid"}};
}

const std::string CheckSize(const std::string& value, std::string_view name,
                            int min, int max) {
  const auto length = icu::UnicodeString::fromUTF8(value).countChar32(0);
  if (length > min && length < max) {
    return value;
  }
  throw errors::ValidationError{
      errors::ErrorBuilder{name, fmt::format("must be longer than {} characters"
                                             " and less than {}",
                                             min, max)}};
}

const std::string CheckSize(const userver::formats::json::Value& data,
                            std::string_view name, int min, int max) {
  if (!data.HasMember(name)) {
    throw errors::ValidationError{errors::ErrorBuilder{name, "required"}};
  }
  const auto& str = data[name].As<std::string>();
  return CheckSize(str, name, min, max);
}

}  // namespace realworld::utils