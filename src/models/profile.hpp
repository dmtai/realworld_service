#pragma once

#include <optional>
#include <string>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>
#include "db/types.hpp"
#include "userver/formats/json.hpp"

namespace realworld::models {

struct Profile final {
  std::string username_;
  std::optional<std::string> bio_;
  std::optional<std::string> image_;
  bool following_{false};
};

}  // namespace realworld::models

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<realworld::models::Profile> {
  static constexpr DBTypeName postgres_name{
      realworld::db::types::kProfile.data()};
};

}  // namespace userver::storages::postgres::io