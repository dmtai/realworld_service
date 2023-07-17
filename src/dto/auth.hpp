#pragma once
#include "userver/formats/json/value.hpp"

namespace realworld::dto {

struct RegistrationRequest final {
  std::string username_;
  std::string password_;
  std::string email_;
};

struct LoginRequest final {
  std::string email_;
  std::string password_;
};

}  // namespace realworld::dto