#include "profile.hpp"

namespace realworld::dto {

userver::formats::json::Value Serialize(
    const Profile& data,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
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
  builder["following"] = data.following_;
  return builder.ExtractValue();
}

}  // namespace realworld::dto