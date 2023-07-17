#pragma once

#include <optional>
#include <string>
#include "models/comment.hpp"
#include "profile.hpp"
#include "userver/formats/json.hpp"

namespace realworld::dto {

struct Comment final {
  static Comment Parse(const models::Comment& model);

  int comment_id;
  std::chrono::system_clock::time_point created_at;
  std::chrono::system_clock::time_point updated_at_;
  std::string body_;
  Profile author_;
};

userver::formats::json::Value Serialize(
    const Comment& Comment,
    userver::formats::serialize::To<userver::formats::json::Value>);

struct NewCommentRequest final {
  std::string body_;
  std::string slug_;
  std::int32_t user_id_;
};

struct DeleteCommentRequest final {
  std::int32_t id_;
  std::string slug_;
};

}  // namespace realworld::dto