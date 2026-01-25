cmake_minimum_required(VERSION 3.14)
project(taskman VERSION 0.7.0 LANGUAGES CXX C)

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

# Catch2 (tests unitaires)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# SQLite amalgamation (third_party/sqlite/)
add_subdirectory(third_party/sqlite)

# -----------------------------------------------------------------------------
# Exécutable taskman
# -----------------------------------------------------------------------------
add_executable(taskman src/main.cpp src/db.cpp src/milestone.cpp src/phase.cpp src/task.cpp)
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

# -----------------------------------------------------------------------------
# Tests unitaires
# -----------------------------------------------------------------------------
enable_testing()
add_executable(tests
  tests/test_db.cpp
  tests/test_milestone.cpp
  tests/test_phase.cpp
  tests/test_task.cpp
  src/db.cpp
  src/milestone.cpp
  src/phase.cpp
  src/task.cpp
)
target_include_directories(tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${SQLITE_AMALGAMATION_SOURCE_DIR}
)
target_link_libraries(tests PRIVATE
  Catch2::Catch2WithMain
  nlohmann_json::nlohmann_json
  cxxopts::cxxopts
  stduuid
  SQLite3
)
if(MSVC)
  target_compile_definitions(tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME UnitTests COMMAND tests)
