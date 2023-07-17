#pragma once

#include <memory>
#include <string>
#include "unicode/translit.h"
#include "userver/formats/json/value.hpp"

namespace realworld::slug {

std::string Slugify(const std::string& str);

}  // namespace realworld::slug