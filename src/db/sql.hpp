#pragma once

#include <string_view>

namespace realworld::db::sql {

inline constexpr std::string_view kAddNewArticle{R"~(
SELECT realworld.add_new_article($1, $2, $3, $4, $5, $6)
)~"};

inline constexpr std::string_view kIsFollowing{R"~(
SELECT realworld.is_following($1, $2)
)~"};

inline constexpr std::string_view kFollow{R"~(
SELECT realworld.follow($1, $2)
)~"};

inline constexpr std::string_view kUnfollow{R"~(
SELECT realworld.unfollow($1, $2)
)~"};

inline constexpr std::string_view kIsFavoritedArticle{R"~(
SELECT realworld.is_favorited_article($1, $2)
)~"};

inline constexpr std::string_view kGetProfile{R"~(
SELECT realworld.get_profile($1, $2)
)~"};

inline constexpr std::string_view kGetProfileByUsername{R"~(
SELECT realworld.get_profile_by_username($1::CITEXT, $2)
)~"};

inline constexpr std::string_view kGetArticleTagList{R"~(
SELECT realworld.get_article_tag_list($1)
)~"};

inline constexpr std::string_view kUpdateArticleBySlug{R"~(
SELECT realworld.update_article_by_slug($1, $2, $3, $4, $5, $6)
)~"};

inline constexpr std::string_view kDeleteArticleBySlug{R"~(
SELECT realworld.delete_article_by_slug($1, $2)
)~"};

inline constexpr std::string_view kGetArticleWithAuthorProfileBySlug{R"~(
SELECT realworld.get_article_with_author_profile_by_slug($1, $2)
)~"};

inline constexpr std::string_view kGetArticleWithAuthorProfile{R"~(
SELECT realworld.get_article_with_author_profile($1, $2)
)~"};

inline constexpr std::string_view kGetArticlesWithAuthorProfile = R"~(
SELECT realworld.get_articles_with_author_profile($1, $2::CITEXT, $3::CITEXT, $4, $5, $6)
)~";

inline constexpr std::string_view kGetFeed{R"~(
SELECT realworld.get_feed($1, $2, $3)
)~"};

inline constexpr std::string_view kAddNewUser{R"~(
SELECT realworld.add_new_user($1::CITEXT, $2, $3)
)~"};

inline constexpr std::string_view kGetUserByEmail{R"~(
SELECT realworld.get_user_by_email($1)
)~"};

inline constexpr std::string_view kGetUserByUsername{R"~(
SELECT realworld.get_user_by_username($1::CITEXT)
)~"};

inline constexpr std::string_view kGetUserById{R"~(
SELECT realworld.get_user_by_id($1)
)~"};

inline constexpr std::string_view kUpdateUserById{R"~(
SELECT realworld.update_user_by_id($1, $2::CITEXT, $3, $4, $5, $6)
)~"};

inline constexpr std::string_view kGetCommentsFromArticle{R"~(
SELECT realworld.get_comments_from_article($1, $2)
)~"};

inline constexpr std::string_view kAddNewComment{R"~(
SELECT realworld.add_comment_to_article($1, $2, $3)
)~"};

inline constexpr std::string_view kDeleteComment{R"~(
SELECT realworld.delete_comment($1, $2, $3)
)~"};

inline constexpr std::string_view kGetComment{R"~(
SELECT realworld.get_comment($1, $2)
)~"};

inline constexpr std::string_view kFavoriteArticle{R"~(
SELECT realworld.favorite_article($1, $2)
)~"};

inline constexpr std::string_view kUnfavoriteArticle{R"~(
SELECT realworld.unfavorite_article($1, $2)
)~"};

inline constexpr std::string_view kGetTags{R"~(
SELECT realworld.get_tags()
)~"};

inline constexpr std::string_view kGetArticleIdBySlug{R"~(
SELECT realworld.get_article_id_by_slug($1)
)~"};

inline constexpr std::string_view kIsCommentExist{R"~(
SELECT realworld.is_comment_exist($1, $2, $3)
)~"};

}  // namespace realworld::db::sql