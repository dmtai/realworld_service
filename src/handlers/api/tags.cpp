#include "tags.hpp"
#include "bcrypt/BCrypt.hpp"
#include "common/auth.hpp"
#include "common/errors.hpp"
#include "common/utils.hpp"
#include "db/sql.hpp"
#include "db/types.hpp"
#include "dto/profile.hpp"
#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"

namespace realworld::handlers::api::tags::get {

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cluster_(context
                   .FindComponent<userver::components::Postgres>(
                       "realworld-database")
                   .GetCluster()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto res =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        db::sql::kGetTags.data());
  const auto tags = res.AsSetOf<std::string>();
  userver::formats::json::ValueBuilder builder;
  builder["tags"] = userver::formats::common::Type::kArray;
  std::for_each(tags.begin(), tags.end(),
                [&builder](const auto& tag) { builder["tags"].PushBack(tag); });
  return builder.ExtractValue();
}

}  // namespace realworld::handlers::api::tags::get