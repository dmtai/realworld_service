#pragma once

#include <optional>
#include <string>
#include <vector>
#include "models/article.hpp"
#include "profile.hpp"
#include "userver/formats/json.hpp"

namespace realworld::dto {

struct Article final {
  static Article Parse(const models::ArticleWithAuthorProfile& model);

  std::string slug_;
  std::string title_;
  std::string description_;
  std::string body_;
  std::optional<std::vector<std::string>> tag_list_;
  std::chrono::system_clock::time_point created_at_;
  std::chrono::system_clock::time_point updated_at_;
  std::int32_t favorites_count_{};
  bool favorited_{false};
  Profile profile_;
};

userver::formats::json::Value Serialize(
    const Article& data,
    userver::formats::serialize::To<userver::formats::json::Value>);

struct NewArticleRequest final {
  std::string title_;
  std::string description_;
  std::string body_;
  std::optional<std::vector<std::string>> tag_list_;
};

struct ArticlesListRequest final {
  std::optional<std::string> tag_;
  std::optional<std::string> author_;
  std::optional<std::string> favorited_;
  std::optional<std::int32_t> limit_;
  std::optional<std::int32_t> offset_;
};

struct FeedRequest final {
  std::optional<std::int32_t> limit_;
  std::optional<std::int32_t> offset_;
};

struct UpdateArticleRequest final {
  std::string slug_;
  std::optional<std::string> title_;
  std::optional<std::string> description_;
  std::optional<std::string> body_;
  std::optional<std::vector<std::string>> tag_list_;
};

}  // namespace realworld::dto