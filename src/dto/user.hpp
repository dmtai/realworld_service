#pragma once

#include <optional>
#include <string>
#include "models/user.hpp"
#include "userver/formats/json.hpp"

namespace realworld::dto {

struct User final {
  std::string email_;
  std::string token_;
  std::string username_;
  std::optional<std::string> bio_{};
  std::optional<std::string> image_{};
};

userver::formats::json::Value Serialize(
    const User& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

struct UpdateUserRequest final {
  std::optional<std::string> email_;
  std::optional<std::string> username_;
  std::optional<std::string> password_;
  std::optional<std::string> bio_;
  std::optional<std::string> image_;
};

}  // namespace realworld::dto