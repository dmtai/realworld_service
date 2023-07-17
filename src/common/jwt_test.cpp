#include "jwt.hpp"
#include <userver/formats/json/serialize_duration.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/utest/utest.hpp>

namespace realworld {

namespace {

constexpr std::int64_t kUserId{666};

userver::formats::json::Value GetTestConfigJson() {
  userver::formats::json::ValueBuilder builder;
  builder["secret_key"] = "secret_key";
  builder["jwt_expiration_time"] = std::chrono::seconds{86400};
  return builder.ExtractValue();
}

}  // namespace

UTEST(JWTManager, GenerateToken) {
  const jwt::JWTConfig config{GetTestConfigJson()};
  const jwt::JWTManager jwt_manager{config};
  const auto token = jwt_manager.GenerateToken(kUserId);
  const auto decoded_token = ::jwt::decode(token);
  const auto id = decoded_token.get_payload_claim("id").as_integer();
  ASSERT_EQ(id, kUserId);
}

UTEST(JWTManager, VerifyToken) {
  const jwt::JWTConfig config{GetTestConfigJson()};
  const jwt::JWTManager jwt_manager{config};
  const auto token = jwt_manager.GenerateToken(kUserId);
  const auto decoded_token = ::jwt::decode(token);
  try {
    jwt_manager.VerifyToken(decoded_token);
  } catch (const std::exception& ex) {
    FAIL() << "JWTManager::VerifyToken failed";
  }
}

}  // namespace realworld