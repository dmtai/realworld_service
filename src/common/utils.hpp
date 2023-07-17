#pragma once

#include <boost/lexical_cast.hpp>
#include <memory>
#include <string>
#include <userver/formats/parse/common_containers.hpp>
#include "errors.hpp"
#include "unicode/translit.h"
#include "userver/formats/json/value.hpp"
#include "userver/server/http/http_request.hpp"

namespace realworld::utils {

const std::string ValidateEmail(const userver::formats::json::Value& data,
                                std::string_view email);

const std::string CheckSize(const std::string& value, std::string_view name,
                            int min, int max);

const std::string CheckSize(const userver::formats::json::Value& data,
                            const std::string_view name, int min, int max);

}  // namespace realworld::utils