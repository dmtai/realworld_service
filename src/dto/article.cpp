#include "article.hpp"

namespace realworld::dto {

Article Article::Parse(const models::ArticleWithAuthorProfile& model) {
  Article article;
  article.slug_ = model.slug_;
  article.title_ = model.title_;
  article.description_ = model.description_;
  article.body_ = model.body_;
  article.tag_list_ = model.tag_list_;
  article.created_at_ = model.created_at_;
  article.updated_at_ = model.updated_at_;
  article.favorites_count_ = model.favorites_count_;
  article.favorited_ = model.favorited_;
  article.profile_.bio_ = model.author_.bio_;
  article.profile_.image_ = model.author_.image_;
  article.profile_.username_ = model.author_.username_;
  article.profile_.following_ = model.author_.following_;
  return article;
}

userver::formats::json::Value Serialize(
    const Article& data,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["slug"] = data.slug_;
  builder["title"] = data.title_;
  builder["description"] = data.description_;
  builder["body"] = data.body_;
  builder["tagList"] = userver::formats::common::Type::kArray;
  if (data.tag_list_) {
    std::for_each(
        data.tag_list_->begin(), data.tag_list_->end(),
        [&builder](const auto& item) { builder["tagList"].PushBack(item); });
  }
  builder["createdAt"] = data.created_at_;
  builder["updatedAt"] = data.updated_at_;
  builder["favoritesCount"] = data.favorites_count_;
  builder["favorited"] = data.favorited_;
  builder["author"] = data.profile_;
  return builder.ExtractValue();
}

}  // namespace realworld::dto