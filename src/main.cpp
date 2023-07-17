#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include "handlers/api/articles.hpp"
#include "handlers/api/articles_feed.hpp"
#include "handlers/api/articles_slug.hpp"
#include "handlers/api/articles_slug_comments.hpp"
#include "handlers/api/articles_slug_favorite.hpp"
#include "handlers/api/articles_slug_unfavorite.hpp"
#include "handlers/api/profiles.hpp"
#include "handlers/api/tags.hpp"
#include "handlers/api/user.hpp"
#include "handlers/api/users.hpp"
#include "handlers/api/users_login.hpp"
#include "handlers/auth/auth.hpp"
#include "userver/clients/dns/component.hpp"

int main(int argc, char* argv[]) {
  using namespace realworld;
  userver::server::handlers::auth::RegisterAuthCheckerFactory(
      "bearer", std::make_unique<handlers::auth::CheckerFactory>());

  const auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .Append<userver::components::HttpClient>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::components::Postgres>("realworld-database")
          .Append<userver::components::Secdist>()
          .Append<userver::components::DefaultSecdistProvider>()
          .Append<userver::clients::dns::Component>()
          .Append<handlers::api::articles::get::Handler>()
          .Append<handlers::api::articles::post::Handler>()
          .Append<handlers::api::articles_feed::get::Handler>()
          .Append<handlers::api::articles_slug::get::Handler>()
          .Append<handlers::api::articles_slug::put::Handler>()
          .Append<handlers::api::articles_slug::del::Handler>()
          .Append<handlers::api::articles_slug_comments::get::Handler>()
          .Append<handlers::api::articles_slug_comments::post::Handler>()
          .Append<handlers::api::articles_slug_comments::del::Handler>()
          .Append<handlers::api::articles_slug_favorite::post::Handler>()
          .Append<handlers::api::articles_slug_unfavorite::del::Handler>()
          .Append<handlers::api::profiles::get::Handler>()
          .Append<handlers::api::profiles::post::Handler>()
          .Append<handlers::api::profiles::del::Handler>()
          .Append<handlers::api::tags::get::Handler>()
          .Append<handlers::api::user::get::Handler>()
          .Append<handlers::api::user::put::Handler>()
          .Append<handlers::api::users::post::Handler>()
          .Append<handlers::api::users_login::post::Handler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}