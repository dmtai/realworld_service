#include "comment.hpp"

namespace realworld::dto {

Comment Comment::Parse(const models::Comment& model) {
  Comment comment;
  comment.body_ = model.body_;
  comment.created_at = model.created_at;
  comment.updated_at_ = model.updated_at_;
  comment.comment_id = model.comment_id;
  comment.author_.bio_ = model.author_.bio_;
  comment.author_.image_ = model.author_.image_;
  comment.author_.username_ = model.author_.username_;
  comment.author_.following_ = model.author_.following_;
  return comment;
}

userver::formats::json::Value Serialize(
    const Comment& data,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["id"] = data.comment_id;
  builder["createdAt"] = data.created_at;
  builder["updatedAt"] = data.updated_at_;
  builder["body"] = data.body_;
  builder["author"] = data.author_;
  return builder.ExtractValue();
}

}  // namespace realworld::dto