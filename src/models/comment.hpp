#pragma once

#include <string>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>
#include "db/types.hpp"
#include "models/profile.hpp"

namespace realworld::models {

struct Comment final {
  int comment_id;
  std::chrono::system_clock::time_point created_at;
  std::chrono::system_clock::time_point updated_at_;
  std::string body_;
  Profile author_;
};

}  // namespace realworld::models

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<realworld::models::Comment> {
  static constexpr DBTypeName postgres_name{
      realworld::db::types::kComment.data()};
};

}  // namespace userver::storages::postgres::io