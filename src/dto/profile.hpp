#pragma once

#include <optional>
#include <string>
#include "userver/formats/json.hpp"

namespace realworld::dto {

struct Profile final {
  std::string username_;
  std::optional<std::string> bio_;
  std::optional<std::string> image_;
  bool following_{false};
};

userver::formats::json::Value Serialize(
    const Profile& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace realworld::dto