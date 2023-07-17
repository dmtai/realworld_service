cmake_minimum_required(VERSION 3.12)
project(realworld_service CXX)

include(third_party/userver/cmake/SetupEnvironment.cmake)
include(GNUInstallDirs)

add_subdirectory(third_party/userver)

option(JWT_BUILD_EXAMPLES OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/jwt-cpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/libbcrypt)

# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    src/common/auth.hpp
    src/common/errors.cpp
    src/common/errors.hpp
    src/common/jwt.cpp
    src/common/jwt.hpp
    src/common/slugify.cpp
    src/common/slugify.hpp
    src/common/utils.cpp
    src/common/utils.hpp
    src/db/sql.hpp
    src/db/types.hpp
    src/dto/article.cpp
    src/dto/article.hpp
    src/dto/auth.hpp
    src/dto/comment.cpp
    src/dto/comment.hpp
    src/dto/profile.cpp
    src/dto/profile.hpp
    src/dto/user.cpp
    src/dto/user.hpp
    src/handlers/api/articles.cpp       
    src/handlers/api/articles_feed.hpp  
    src/handlers/api/articles_slug_comments.cpp  
    src/handlers/api/articles_slug.cpp           
    src/handlers/api/articles_slug_favorite.hpp  
    src/handlers/api/articles_slug_unfavorite.cpp  
    src/handlers/api/profiles.cpp  
    src/handlers/api/tags.cpp  
    src/handlers/api/user.cpp  
    src/handlers/api/users.cpp  
    src/handlers/api/users_login.cpp
    src/handlers/api/articles_feed.cpp  
    src/handlers/api/articles.hpp       
    src/handlers/api/articles_slug_comments.hpp  
    src/handlers/api/articles_slug_favorite.cpp  
    src/handlers/api/articles_slug.hpp           
    src/handlers/api/articles_slug_unfavorite.hpp  
    src/handlers/api/profiles.hpp  
    src/handlers/api/tags.hpp  
    src/handlers/api/user.hpp  
    src/handlers/api/users.hpp  
    src/handlers/api/users_login.hpp
    src/handlers/auth/auth.cpp
    src/handlers/auth/auth.hpp
    src/models/article.hpp
    src/models/comment.hpp
    src/models/profile.hpp
    src/models/user.hpp
)
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver-core userver-postgresql)

target_include_directories(${PROJECT_NAME}_objs PUBLIC jwt-cpp)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC jwt-cpp)

target_include_directories(${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbcrypt/include)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC bcrypt)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
find_package(ICU 4.4 COMPONENTS io i18n REQUIRED)
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${ICU_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${ICU_LIBRARIES})

# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Unit Tests
add_executable(${PROJECT_NAME}_unittest
    src/common/jwt_test.cpp
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver-utest)
add_google_tests(${PROJECT_NAME}_unittest)

# Functional Tests
add_subdirectory(tests)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX ${PREFIX_PATH})
endif()

set(CONFIG_VAR_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/config_vars.yaml)
set(CONFIG_FALLBACK_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/dynamic_config_fallback.json)
set(CONFIG_JWT ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/jwt_config.json)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml)

FILE(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})