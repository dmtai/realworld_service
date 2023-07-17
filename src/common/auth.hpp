#pragma once

#include <common/jwt.hpp>

namespace realworld::auth {

struct UserAuthData final {
  std::int32_t id_{};
  jwt::DecodedToken token_;
};

}  // namespace realworld::auth