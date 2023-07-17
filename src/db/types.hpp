#pragma once

#include <string_view>

namespace realworld::db::types {

inline constexpr std::string_view kUser{"realworld.realworld_user"};

inline constexpr std::string_view kProfile{"realworld.profile"};

inline constexpr std::string_view kComment{"realworld.realworld_comment"};

inline constexpr std::string_view kArticleWithAuthorProfile{
    "realworld.article_with_author_profile"};

}  // namespace realworld::db::types