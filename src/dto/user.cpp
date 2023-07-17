#include "user.hpp"

namespace realworld::dto {

userver::formats::json::Value Serialize(
    const User& data,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["email"] = data.email_;
  builder["token"] = data.token_;
  builder["username"] = data.username_;
  if (data.bio_) {
    builder["bio"] = *data.bio_;
  } else {
    builder["bio"] = userver::formats::common::Type::kNull;
  }
  if (data.image_) {
    builder["image"] = *data.image_;
  } else {
    builder["image"] = userver::formats::common::Type::kNull;
  }
  return builder.ExtractValue();
}

}  // namespace realworld::dto