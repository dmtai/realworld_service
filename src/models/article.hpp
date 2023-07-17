#pragma once

#include <chrono>
#include <cstdint>
#include <db/types.hpp>
#include <models/user.hpp>
#include <optional>
#include <string>
#include <userver/storages/postgres/io/pg_types.hpp>
#include <vector>
#include "profile.hpp"

namespace realworld::models {

struct ArticleWithAuthorProfile final {
  int article_id_;
  std::string title_;
  std::string slug_;
  std::string description_;
  std::string body_;
  std::chrono::system_clock::time_point created_at_;
  std::chrono::system_clock::time_point updated_at_;
  std::optional<std::vector<std::string>> tag_list_;
  bool favorited_;
  std::int64_t favorites_count_;
  Profile author_;
};

}  // namespace realworld::models

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<realworld::models::ArticleWithAuthorProfile> {
  static constexpr DBTypeName postgres_name{
      realworld::db::types::kArticleWithAuthorProfile.data()};
};

}  // namespace userver::storages::postgres::io