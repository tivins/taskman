cmake_minimum_required(VERSION 3.14)
file(READ "${CMAKE_SOURCE_DIR}/VERSION" TASKMAN_VERSION_RAW)
string(STRIP "${TASKMAN_VERSION_RAW}" TASKMAN_VERSION)
project(taskman VERSION ${TASKMAN_VERSION} LANGUAGES CXX C)

# version.h from VERSION file (for TASKMAN_VERSION in C++)
configure_file(
  ${CMAKE_SOURCE_DIR}/src/version.h.in
  ${CMAKE_BINARY_DIR}/version.h
)

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

# cpp-httplib (HTTP server, header-only by default)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.14.3
)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "")
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "")
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "")
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE BOOL "")
set(HTTPLIB_INSTALL OFF CACHE BOOL "")
set(HTTPLIB_TEST OFF CACHE BOOL "")
FetchContent_MakeAvailable(httplib)

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
# Web assets: style.css, multiple JS files -> web_assets.generated.h (embedded in binary)
# -----------------------------------------------------------------------------
find_package(Python3 REQUIRED)
set(WEB_ASSETS_HEADER "${CMAKE_BINARY_DIR}/web_assets.generated.h")
add_custom_command(
  OUTPUT "${WEB_ASSETS_HEADER}"
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/embed_assets.py
    --css ${CMAKE_SOURCE_DIR}/embed/web/style.css
    --js ${CMAKE_SOURCE_DIR}/embed/web/dom.js
    --js ${CMAKE_SOURCE_DIR}/embed/web/filters.js
    --js ${CMAKE_SOURCE_DIR}/embed/web/pagination.js
    --js ${CMAKE_SOURCE_DIR}/embed/web/main.js
    -o "${WEB_ASSETS_HEADER}"
  DEPENDS ${CMAKE_SOURCE_DIR}/scripts/embed_assets.py
          ${CMAKE_SOURCE_DIR}/embed/web/style.css
          ${CMAKE_SOURCE_DIR}/embed/web/dom.js
          ${CMAKE_SOURCE_DIR}/embed/web/filters.js
          ${CMAKE_SOURCE_DIR}/embed/web/pagination.js
          ${CMAKE_SOURCE_DIR}/embed/web/main.js
  COMMENT "Embedding web assets (style.css, dom.js, filters.js, pagination.js, main.js) into header"
)

# -----------------------------------------------------------------------------
# Agent files: embed/roles_agents/*.md -> agents.generated.h (embedded in binary)
# -----------------------------------------------------------------------------
file(GLOB AGENT_MD_FILES "${CMAKE_SOURCE_DIR}/embed/roles_agents/*.md")
set(AGENTS_HEADER "${CMAKE_BINARY_DIR}/agents.generated.h")
add_custom_command(
  OUTPUT "${AGENTS_HEADER}"
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/embed_agents.py
    --agents-dir ${CMAKE_SOURCE_DIR}/embed/roles_agents
    -o "${AGENTS_HEADER}"
  DEPENDS ${CMAKE_SOURCE_DIR}/scripts/embed_agents.py
          ${AGENT_MD_FILES}
  COMMENT "Embedding agent files (embed/roles_agents/*.md) into header"
)

# -----------------------------------------------------------------------------
# Rule files: embed/rules/*.mdc -> rules.generated.h (embedded in binary)
# -----------------------------------------------------------------------------
file(GLOB RULE_FILES "${CMAKE_SOURCE_DIR}/embed/rules/*.mdc" "${CMAKE_SOURCE_DIR}/embed/rules/*.md")
set(RULES_HEADER "${CMAKE_BINARY_DIR}/rules.generated.h")
add_custom_command(
  OUTPUT "${RULES_HEADER}"
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/embed_rules.py
    --rules-dir ${CMAKE_SOURCE_DIR}/embed/rules
    -o "${RULES_HEADER}"
  DEPENDS ${CMAKE_SOURCE_DIR}/scripts/embed_rules.py
          ${RULE_FILES}
  COMMENT "Embedding rule files (embed/rules/*.mdc) into header"
)

# -----------------------------------------------------------------------------
# Exécutable taskman
# -----------------------------------------------------------------------------
add_executable(taskman
  # Point d'entrée
  src/main.cpp
  
  # Core - Task
  src/core/task/task.cpp
  src/core/task/task_repository.cpp
  src/core/task/task_service.cpp
  src/core/task/task_formatter.cpp
  src/core/task/task_command_parser.cpp
  
  # Core - Phase
  src/core/phase/phase.cpp
  src/core/phase/phase_repository.cpp
  src/core/phase/phase_service.cpp
  src/core/phase/phase_formatter.cpp
  src/core/phase/phase_command_parser.cpp
  
  # Core - Milestone
  src/core/milestone/milestone.cpp
  src/core/milestone/milestone_repository.cpp
  src/core/milestone/milestone_service.cpp
  src/core/milestone/milestone_formatter.cpp
  src/core/milestone/milestone_command_parser.cpp
  
  # Core - Note
  src/core/note/note.cpp
  src/core/note/note_repository.cpp
  src/core/note/note_service.cpp
  src/core/note/note_formatter.cpp
  src/core/note/note_command_parser.cpp
  
  # Infrastructure - Database
  src/infrastructure/db/db_connection.cpp
  src/infrastructure/db/query_executor.cpp
  src/infrastructure/db/schema_manager.cpp
  
  # CLI
  src/cli/command.cpp
  src/cli/commands.cpp
  
  # Web
  src/web/web.cpp
  src/web/web_server.cpp
  src/web/web_controllers.cpp
  
  # MCP
  src/mcp/mcp.cpp
  src/mcp/mcp_config.cpp
  src/mcp/mcp_protocol_handler.cpp
  src/mcp/mcp_tool_registry.cpp
  src/mcp/mcp_tool_executor.cpp
  
  # Util
  src/util/agents.cpp
  src/util/demo.cpp
  src/util/formats.cpp
  src/util/roles.cpp
  src/util/rules.cpp

  # Assets générés
  "${WEB_ASSETS_HEADER}"
  "${AGENTS_HEADER}"
  "${RULES_HEADER}"
)
if(MSVC)
  target_compile_definitions(taskman PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_include_directories(taskman PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR} ${SQLITE_AMALGAMATION_SOURCE_DIR})
target_link_libraries(taskman PRIVATE
  nlohmann_json::nlohmann_json
  cxxopts::cxxopts
  stduuid
  SQLite3
  httplib::httplib
)

# -----------------------------------------------------------------------------
# Tests unitaires
# -----------------------------------------------------------------------------
enable_testing()
add_executable(tests
  tests/test_db.cpp
  tests/test_integration.cpp
  tests/test_mcp.cpp
  tests/test_milestone.cpp
  tests/test_note.cpp
  tests/test_phase.cpp
  tests/test_task.cpp
  
  # Core - Task
  src/core/task/task.cpp
  src/core/task/task_repository.cpp
  src/core/task/task_service.cpp
  src/core/task/task_formatter.cpp
  src/core/task/task_command_parser.cpp
  
  # Core - Phase
  src/core/phase/phase.cpp
  src/core/phase/phase_repository.cpp
  src/core/phase/phase_service.cpp
  src/core/phase/phase_formatter.cpp
  src/core/phase/phase_command_parser.cpp
  
  # Core - Milestone
  src/core/milestone/milestone.cpp
  src/core/milestone/milestone_repository.cpp
  src/core/milestone/milestone_service.cpp
  src/core/milestone/milestone_formatter.cpp
  src/core/milestone/milestone_command_parser.cpp
  
  # Core - Note
  src/core/note/note.cpp
  src/core/note/note_repository.cpp
  src/core/note/note_service.cpp
  src/core/note/note_formatter.cpp
  src/core/note/note_command_parser.cpp
  
  # Infrastructure - Database
  src/infrastructure/db/db_connection.cpp
  src/infrastructure/db/query_executor.cpp
  src/infrastructure/db/schema_manager.cpp
  
  # Util
  src/util/formats.cpp
  src/util/roles.cpp
)
target_include_directories(tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}
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
  target_compile_options(tests PRIVATE /FS)
endif()
add_dependencies(tests taskman)
add_test(NAME UnitTests COMMAND tests)
