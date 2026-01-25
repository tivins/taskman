cmake_minimum_required(VERSION 3.14)
project(taskman VERSION 0.3.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Dépendances via FetchContent
# -----------------------------------------------------------------------------
include(FetchContent)

# nlohmann/json (header-only)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
set(JSON_Install OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(json)

# cxxopts (header-only)
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.1.0
)
FetchContent_MakeAvailable(cxxopts)

# stduuid (header-only, UUID v4)
FetchContent_Declare(
  stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
  GIT_TAG        v1.2.2
)
set(UUID_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(UUID_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(stduuid)

# SQLite amalgamation (third_party/sqlite/)
add_subdirectory(third_party/sqlite)

# -----------------------------------------------------------------------------
# Exécutable taskman
# -----------------------------------------------------------------------------
add_executable(taskman src/main.cpp src/db.cpp src/phase.cpp)
if(MSVC)
  target_compile_definitions(taskman PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_include_directories(taskman PRIVATE ${SQLITE_AMALGAMATION_SOURCE_DIR})
target_link_libraries(taskman PRIVATE
  nlohmann_json::nlohmann_json
  cxxopts::cxxopts
  stduuid
  SQLite3
)
