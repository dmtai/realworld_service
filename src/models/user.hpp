#pragma once

#include <optional>
#include <string>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>
#include "db/types.hpp"

namespace realworld::models {

struct User final {
  std::int32_t id_;
  std::string username_;
  std::string email_;
  std::string hash_;
  std::optional<std::string> bio_;
  std::optional<std::string> image_;
};

}  // namespace realworld::models

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<realworld::models::User> {
  static constexpr DBTypeName postgres_name{realworld::db::types::kUser.data()};
};

}  // namespace userver::storages::postgres::io