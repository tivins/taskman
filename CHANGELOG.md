# Changelog

## [0.34.0] - 2026-02-01

### Added

- **UI web — Étape 4 (liste améliorée et navigation)** : recherche globale dans le header (titre, description, ID — filtrage côté client temporaire, ADR-0003) ; tri par colonne (titre, statut, rôle, phase, jalon, mis à jour) avec ordre asc/desc ; groupement par phase, jalon, statut ou rôle avec sections repliables ; persistance de l’état dans l’URL (ADR-0002) : filtres, page, recherche, tri et groupement reflétés dans `#/list?page=...&phase=...&search=...&sort=...&order=...&group_by=...`, restauration au chargement et sur back/forward. Module `embed/web/url-state.js` pour lecture/écriture du hash.

---

## [0.33.0] - 2026-02-01

### Added

- **UI web — Sidebar gauche (étape 2.4)** : barre latérale repliable avec liens rapides (Tâches, Vue d’ensemble) et arborescence phases / jalons. Un clic sur une phase ou un jalon applique le filtre correspondant et affiche la liste des tâches. État replié persisté dans `localStorage`. Préparation pour l’étape 4 (liste améliorée et navigation).

---

## [0.32.1] - 2026-02-01

### Changed

- **UI web — Étape 1 (corrections et cohérence)** : liste des rôles du filtre alignée sur le backend (14 rôles, `src/util/roles.cpp`) ; filtre « Blocage » exposé dans la zone filtres (`blocked_filter=blocked|unblocked`) ; bouton « Réinitialiser » les filtres réactivé (`embed/web/filters.js`).

---

## [0.32.0] - 2026-01-31

### Added

- **Filtre tâches bloquées** : possibilité de filtrer la liste des tâches par état bloqué (tâche ayant au moins une dépendance non-`done`). CLI : `task:list --blocked-filter blocked|unblocked` ; API web : paramètre `blocked_filter` sur GET /tasks et GET /tasks/count ; MCP : paramètre `blocked-filter` sur `taskman_task_list`. Pas de nouvelle colonne : calcul SQL via `task_deps` et statut des dépendances.

---

## [0.31.0] - 2026-01-31

### Added

- **Créateur (rôle) de la tâche** : colonne `creator` en base (migration automatique), propagation dans add/edit/get/list et formatters. Traçabilité : qui a créé la tâche (rôle, utile pour audit et agents). CLI : `task:add --creator <role>`, `task:edit --creator <role>` ; MCP : paramètre `creator` sur `taskman_task_add` et `taskman_task_edit` ; API web : champ `creator` dans les réponses GET /task/:id et GET /tasks. Le créateur est validé comme rôle (même liste que `role`).

---

## [0.30.0] - 2026-01-31

### Added

- **task:note:list-by-ids** : récupération d'une liste de notes par liste d'UID (`--ids id1,id2,...`). Permet d'exploiter les `note_ids` obtenus via `task:get` en un seul appel au lieu de N appels `get_note(id)`. Commande CLI, outil MCP `taskman_task_note_list_by_ids`, API web `GET /notes?ids=...`, et API service/repository (`list_notes_by_ids` / `list_by_ids`).
- **API web** : endpoint `GET /notes?ids=id1,id2,...` pour récupérer plusieurs notes en une requête. Documentation `docs/usage_web.md` mise à jour (endpoint, section Features).

---

## [0.29.0] - 2026-01-31

### Added

- **task:get** : la récupération d'une tâche inclut désormais la liste des UID des notes liées (`note_ids`). En JSON : tableau `note_ids` ; en `--format text` : ligne `note_ids: id1,id2,...`. Prérequis pour afficher une tâche avec ses notes sans appel séparé (CLI, MCP, web).

---

## [0.28.2] - 2026-01-30

### Added

- **embed/rules/design-patterns.mdc**: Règle à l’intention des développeurs pour utiliser des design patterns lorsque cela est justifié (Strategy, Repository, Factory, etc.), avec critères d’usage et exemples à éviter sans justification.

---

## [0.28.1] - 2026-01-29

### Added

- **get_executable_path()**: Utility (Windows/Linux/macOS) to obtain the absolute path of the running executable. Used by `project:init` so `mcp:config` runs without requiring `--executable`.

### Changed

- **project:init**: Now uses the detected executable path by default; `--executable` is only needed to override. A single run of `taskman project:init` (or `/path/to/taskman project:init`) generates `.cursor/mcp.json` automatically.
- **Documentation**: Quick start shows `/path/to/taskman project:init` to indicate taskman is not in PATH; mcp.json is generated without `--executable`.

### Fixed

- **Windows**: `get_executable_path()` returned empty when the executable path exceeded MAX_PATH (260 characters). Now uses a larger buffer (32768) so long paths (e.g. under `.cursor/projects/...`) are resolved correctly and mcp:config is no longer skipped.

---

## [0.28.0] - 2026-01-29

### Added

- **project:init**: Bootstrap command that runs in order: `mcp:config` (if `--executable` given), `init`, `rules:generate`, `agents:generate`. Use to set up a new project for Cursor and the agent; then reload Cursor.
- **MCP tools** for rules and agents: `taskman_rules_generate`, `taskman_agents_generate` (optional `output`), and `taskman_project_init` (optional `executable`). Rules and agents generation is now available via MCP so the agent can "generate rules" or "retrieve agents" on request.
- **MCP executor**: Opens the database only when the command requires it, so `taskman_rules_generate` and `taskman_agents_generate` work before the project DB exists.

### Changed

- **Documentation**: README and docs (usage_mcp.md, usage.md, usage_cli.md) updated with a 4-step quick start: download taskman → `/path/to/taskman project:init` → reload Cursor → use Taskman via the agent.

---

## [0.27.0] - 2026-01-29

### Added

- **rules:generate**: commande CLI et pipeline d’embed pour les rules Cursor, symétrique à `agents:generate`. Les fichiers dans `embed/rules/` (`.mdc` et `.md`) sont intégrés au binaire via `scripts/embed_rules.py` → `rules.generated.h`, et la commande `taskman rules:generate [--output <dir>]` les écrit dans `.cursor/rules/` (ou le répertoire spécifié). Fichiers ajoutés : `scripts/embed_rules.py`, `src/util/rules.hpp`, `src/util/rules.cpp` ; CMake : génération de `rules.generated.h` et lien avec l’exécutable taskman.

## [0.26.0] - 2026-01-28

### Changed

- **Réorganisation de la structure des dossiers source**: réorganisation complète du dossier `src/` en une structure modulaire avec séparation claire des responsabilités :
  - **`src/core/`** : entités métier (domain layer) organisées par module :
    - `core/task/` : tâches (task.hpp, task_repository.hpp, task_service.hpp, task_formatter.hpp, task_command_parser.hpp)
    - `core/phase/` : phases (phase.hpp, phase_repository.hpp, phase_service.hpp, phase_formatter.hpp, phase_command_parser.hpp)
    - `core/milestone/` : milestones (milestone.hpp, milestone_repository.hpp, milestone_service.hpp, milestone_formatter.hpp, milestone_command_parser.hpp)
    - `core/note/` : notes (note.hpp, note_repository.hpp, note_service.hpp, note_formatter.hpp, note_command_parser.hpp)
  - **`src/infrastructure/`** : infrastructure technique :
    - `infrastructure/db/` : couche base de données (db.hpp, db_connection.hpp, query_executor.hpp, schema_manager.hpp)
  - **`src/cli/`** : interface ligne de commande (command.hpp, commands.cpp)
  - **`src/web/`** : interface web (web.hpp, web_server.hpp, web_controllers.hpp)
  - **`src/mcp/`** : interface MCP (mcp.hpp, mcp_config.hpp, mcp_protocol_handler.hpp, mcp_tool_registry.hpp, mcp_tool_executor.hpp)
  - **`src/util/`** : utilitaires (formats.hpp, roles.hpp, agents.hpp, demo.hpp)
- Tous les includes ont été mis à jour pour utiliser les nouveaux chemins relatifs depuis `src/`
- `CMakeLists.txt` a été mis à jour avec les nouveaux chemins, organisés par catégorie pour une meilleure lisibilité
- Les include directories ont été configurés pour permettre l'utilisation des chemins depuis `src/`
- Cette réorganisation améliore la navigation dans le code, facilite la maintenance et prépare le projet pour une éventuelle introduction de namespaces dans le futur

## [0.25.0] - 2026-01-28

### Changed

- **SOLID Refactoring - phase/milestone/note modules (item 1.2 completion)**: completion of the refactoring following the SRP (Single Responsibility Principle) for the phase, milestone, and note modules. These modules have been refactored to separate responsibilities into specialized classes:
  - **Phase module**: `PhaseService` (business logic and validation), `PhaseFormatter` (output formatting), `PhaseCommandParser` (CLI parsing), `PhaseRepository` extended with `add()` and `update()` methods
  - **Milestone module**: `MilestoneService` (business logic and validation), `MilestoneFormatter` (output formatting), `MilestoneCommandParser` (CLI parsing), `MilestoneRepository` extended with `add()`, `update()`, and `list_by_phase()` methods
  - **Note module**: `NoteService` (business logic, validation, UUID generation), `NoteFormatter` (output formatting), `NoteCommandParser` (CLI parsing), `NoteRepository` extended with `add()`, `get_by_id()`, and `task_exists()` methods
- The `cmd_phase_*`, `cmd_milestone_*`, and `cmd_note_*` functions are now wrappers using the new classes, maintaining compatibility with existing code (`main.cpp`, `mcp.cpp`)
- The code now fully complies with the SRP principle: each class has a single, clear responsibility, matching the pattern established for the Task module in v0.20.0

### Added

- **New files**: `phase_service.hpp`/`phase_service.cpp`, `phase_formatter.hpp`/`phase_formatter.cpp`, `phase_command_parser.hpp`/`phase_command_parser.cpp`, `milestone_service.hpp`/`milestone_service.cpp`, `milestone_formatter.hpp`/`milestone_formatter.cpp`, `milestone_command_parser.hpp`/`milestone_command_parser.cpp`, `note_service.hpp`/`note_service.cpp`, `note_formatter.hpp`/`note_formatter.cpp`, `note_command_parser.hpp`/`note_command_parser.cpp`

## [0.24.0] - 2026-01-28

### Changed

- **SOLID Refactoring - mcp.cpp (item 1.5)**: separation of responsibilities following the SRP (Single Responsibility Principle). The `mcp.cpp` file has been refactored to separate the JSON-RPC protocol from business logic and tool execution:
  - `McpProtocolHandler`: dedicated handling of the JSON-RPC protocol (parsing, validation, response construction, JSON-RPC error handling)
  - `McpToolRegistry`: dedicated definition of MCP tools (JSON schemas, descriptions, mapping to CLI commands)
  - `McpToolExecutor`: dedicated execution of MCP tools (JSON → argv conversion, execution via CommandRegistry, capture of stdout/stderr)
- Significant reduction in `mcp.cpp` size: from ~710 lines to ~100 lines (initialization + main loop)
- Reuse of the existing `CommandRegistry` for CLI command execution, avoiding code duplication
- The code now complies with the SRP principle: each class has a single, clear responsibility
- Fixed error handling for unknown tools: sends an appropriate JSON-RPC error (code -32602) instead of a normal response with `isError = true`

### Added

- **New files**: `mcp_protocol_handler.hpp`/`mcp_protocol_handler.cpp` (JSON-RPC protocol handler), `mcp_tool_registry.hpp`/`mcp_tool_registry.cpp` (MCP tools registry), `mcp_tool_executor.hpp`/`mcp_tool_executor.cpp` (MCP tools executor)

## [0.23.0] - 2026-01-28

### Changed

- **SOLID Refactoring - web.cpp (item 1.4)**: separation of responsibilities following the SRP (Single Responsibility Principle). The `web.cpp` file has been refactored to separate HTTP infrastructure from business logic:
  - `WebServer`: dedicated HTTP infrastructure (startup, configuration, basic routing, static assets)
  - `TaskController`: dedicated REST endpoints for tasks (`/task/:id`, `/tasks`, `/task/:id/deps`, `/task/:id/notes`, `/tasks/count`, `/task_deps`)
  - `PhaseController`: dedicated REST endpoints for phases (`/phase/:id`, `/phases`)
  - `MilestoneController`: dedicated REST endpoints for milestones (`/milestone/:id`, `/milestones`)
  - `PhaseRepository`, `MilestoneRepository`, `NoteRepository`: repositories for accessing phase, milestone, and note data
- Controllers reuse existing business services (`TaskService`, `TaskRepository`) and newly created repositories
- `TaskRepository` has been extended with methods for pagination and counting (`list_paginated()`, `count()`, `get_dependencies()`, `list_dependencies()`)
- The code now complies with the SRP principle: each class has a single, clear responsibility

### Added

- **New files**: `web_server.hpp`/`web_server.cpp` (HTTP infrastructure), `web_controllers.hpp`/`web_controllers.cpp` (REST controllers), `phase_repository.hpp`/`phase_repository.cpp`, `milestone_repository.hpp`/`milestone_repository.cpp`, `note_repository.hpp`/`note_repository.cpp`

## [0.22.0] - 2026-01-28

### Changed

- **SOLID Refactoring - main.cpp (item 1.3)**: implementation of the Command pattern for CLI command routing, respecting SRP (Single Responsibility Principle) and OCP (Open/Closed Principle). The `main.cpp` file has been refactored to use a `CommandRegistry` instead of a long if/else chain:
  - `Command`: abstract interface for all CLI commands (`execute()`, `name()`, `summary()`, `requires_database()`)
  - `CommandRegistry`: centralized registry allowing registration and execution of commands by name
  - Concrete command classes: wrappers for each existing command (`InitCommand`, `PhaseAddCommand`, `TaskAddCommand`, etc.)
  - `register_all_commands()`: centralized function registering all available commands
- New commands can now be added without modifying `main.cpp`: just create a class inheriting from `Command` and register it in `register_all_commands()`
- Help generation (`taskman -h`) now uses the command registry to dynamically list all available commands
- Optimized database management: opened only for commands that need it (via `requires_database()`)

### Added

- **New files**: `command.hpp`/`command.cpp` (Command interface and CommandRegistry), `commands.cpp` (implementations for all command classes)

## [0.21.0] - 2026-01-28

### Changed

- **SOLID Refactoring - Task Module (item 1.2)**: separation of responsibilities following the SRP (Single Responsibility Principle). The `task.cpp` module has been refactored into four specialized classes:
  - `TaskRepository`: exclusive database access for tasks (CRUD, dependencies)
  - `TaskService`: exclusive business logic (validation, UUID generation, business rules)
  - `TaskFormatter`: exclusive output formatting (JSON, text)
  - `TaskCommandParser`: exclusive CLI parsing (cxxopts)
- The `cmd_task_*` functions are now wrappers using the new classes, maintaining compatibility with the existing code (`main.cpp`, `mcp.cpp`)
- Added `Database::get_executor()` to allow repositories to access QueryExecutor

### Added

- **New files**: `task_repository.hpp`/`task_repository.cpp`, `task_service.hpp`/`task_service.cpp`, `task_formatter.hpp`/`task_formatter.cpp`, `task_command_parser.hpp`/`task_command_parser.cpp`

## [0.20.0] - 2026-01-28

### Changed

- **Project structure reorganization**: moved files embedded in the binary into a dedicated `embed/` folder to enable use of namespaces in `src/`:
  - `src/web/` → `embed/web/` (web assets: CSS and JavaScript files)
  - `docs/roles_agents/` → `embed/roles_agents/` (markdown files for agents)
- Updated `CMakeLists.txt`, `scripts/embed_assets.py`, `scripts/embed_agents.py`, `src/web.cpp` and `docs/usage_web.md` to use the new paths

## [0.19.0] - 2026-01-28

### Changed

- **SOLID Refactoring - Database class**: separation of responsibilities following the SRP (Single Responsibility Principle). The `Database` class has been refactored into three specialized classes:
  - `DatabaseConnection`: dedicated SQLite connection management (open, close, is_open)
  - `QueryExecutor`: dedicated SQL query execution (exec, run, query)
  - `SchemaManager`: dedicated schema and migration management (init_schema, table_has_column, ensure_timestamps)
- The `Database` class is now a facade that delegates to the three specialized classes, maintaining compatibility with existing code while respecting SOLID principles

### Added

- **New files**: `db_connection.hpp`/`db_connection.cpp`, `query_executor.hpp`/`query_executor.cpp`, `schema_manager.hpp`/`schema_manager.cpp`

## [0.18.1] - 2026-01-28

### Added

- **Web view**: GET route `/task/:id/notes` and a "Note history" section in the task detail view, to consult notes added by the agent during execution (id, content, kind, role, created_at)

## [0.18.0] - 2026-01-27

### Added

- **Task notes**: table `task_notes` (id, task_id, content, kind, role, created_at), created by `init_schema`. Reusable `note_add()` function and CLI commands `task:note:add <task-id> --content "..." [--kind completion|progress|issue] [--role <role>] [--format json|text]` and `task:note:list <task-id> [--format json|text]`. MCP tools `taskman_task_note_add` and `taskman_task_note_list`. Task must exist; `kind` and `role` are optional. Output: created note (add) or list ordered by `created_at` (list).
- **demo:generate**: adds 7 notes (completion / progress) to demo tasks (t1, t2, t3, t4, t5, t7, t15)

### Changed

- **note_add**: when adding a note, the `updated_at` of the linked task is updated (`UPDATE tasks SET updated_at = datetime('now') WHERE id = ?`)

## [0.17.1] - 2026-01-27

### Fixed

- **Build (MSVC)**: `scripts/embed_assets.py` now splits string literals (CSS/JS) into blocks of at most 16,000 characters to stay within the MSVC limit (~16,380, error C2026). Blocks are emitted as concatenated literals. Fixes Windows/MSVC compilation.

## [0.17.0] - 2026-01-27

### Added

- **created_at, updated_at** on the tables `phases`, `milestones`, `tasks`: automatic columns (user not allowed to modify). `created_at` and `updated_at` are set during insertion via `DEFAULT (datetime('now'))`. On every update (phase:edit, milestone:edit, task:edit), `updated_at` is refreshed. Automatic migration for existing DBs (adds the columns if absent). These fields are exposed in JSON and text format on all channels (CLI, WEB, MCP).

### Changed

- **Schema**: `init_schema` creates tables with `created_at` and `updated_at`; a migration adds these columns to existing tables via `pragma_table_info` and `ALTER TABLE ... ADD COLUMN`
- **Formats**: `phase_to_json`, `milestone_to_json`, `task_to_json`, and `print_task_text` include `created_at` and `updated_at`

## [0.16.0] - 2026-01-27

### Changed

- **Phase/milestone/task utilities refactoring**: The commands `phase:add`, `milestone:add`, `task:add`, `task:dep:add`, and `demo:generate` now rely on reusable utility functions (`phase_add`, `milestone_add`, `task_add`, `task_dep_add`) declared in headers. This avoids duplication of SQL and validation logic, centralizes rules (status, reached, roles), and allows programmatic usage without artificially constructing `argv`. See `docs/REFACTORING.md`.
- **demo:generate** uses `phase_add`, `milestone_add`, `task_add`, and `task_dep_add` instead of direct SQL queries.

### Added

- **phase_add(Database&, id, name, status?, sort_order?)** in `phase.hpp`: adds a phase with status validation
- **milestone_add(Database&, id, phase_id, name?, criterion?, reached)** in `milestone.hpp`: adds a milestone
- **task_add(Database&, id, phase_id, milestone_id?, title, description?, status?, sort_order?, role?)** in `task.hpp`: adds a task with status and role validation
- **task_dep_add(Database&, task_id, depends_on)** in `task.hpp`: adds a dependency between tasks (checks existence and no self-dependency)
- Direct unit tests for these functions in `tests/test_phase.cpp`, `tests/test_milestone.cpp`, `tests/test_task.cpp`

## [0.15.0] - 2026-01-27

### Added

- **taskman demo:generate**: built-in command to create a demonstration database filled with a realistic example (e-commerce MVP project). Replaces the Python script `scripts/create_demo.py` and allows direct use of functions from the C++ codebase without providing the path of the executable. The command automatically deletes the existing database (and its journal files) before creating the new one, ensuring a reproducible result. Creates 4 phases (Design, Development, Acceptance, Delivery), 4 milestones, 9 tasks with priorities (sort_order), and their dependencies.
- **src/demo.cpp**, **src/demo.hpp**: module implementing the `demo:generate` command with direct data creation through SQL

## [0.14.0] - 2026-01-27

### Added

- **taskman mcp:config** [--executable <path>] [--output <file>]: generates or updates the `.cursor/mcp.json` file with the taskman MCP server configuration. Takes the absolute path of the taskman executable (or taskman.exe) as parameter. For `TASKMAN_DB_NAME`, uses current directory + "project_tasks.db". Merges with existing MCP servers in the file. Similar to `agents:generate` for MCP configuration.
- **src/mcp_config.cpp**, **src/mcp_config.hpp**: module implementing the `mcp:config` command with JSON file manipulation (read, merge, write)

## [0.13.0] - 2026-01-27

### Added

- **taskman agents:generate** [--output <dir>]: generates agent files in `.cursor/agents/` (or the specified directory) from markdown files embedded in the binary. Source files are located in `docs/roles_agents/*.md` and are embedded in the binary during compilation via `scripts/embed_agents.py`. The command extracts these files and writes them into the output directory.
- **scripts/embed_agents.py**: Python script that generates `agents.generated.h` from markdown files in `docs/roles_agents/`. Files are embedded as raw string literals in the generated header.
- **src/agents.cpp**, **src/agents.hpp**: module implementing the `agents:generate` command with filesystem handling (directory creation, file writing)
- **CMakeLists.txt**: added generation of `agents.generated.h` via `add_custom_command` with dependencies on all `.md` files in `docs/roles_agents/`

## [0.12.0] - 2026-01-25

### Added

- **taskman mcp**: MCP server mode (Model Context Protocol) over stdio. Reads JSON-RPC requests line-by-line from stdin, writes responses to stdout. Full MCP protocol implementation as of 2025-11-25:
  - Lifecycle: `initialize` (with `protocolVersion` validation), `notifications/initialized`
  - Methods: `tools/list` (returns all 13 tools), `tools/call` (executes CLI commands), `ping`
  - Table of 13 tools: full definitions with `name`, `description`, `inputSchema` (JSON Schema), `positional_keys`
  - Arguments conversion → `argv`: automatic CLI argument construction from JSON parameters (positional, then options as `--key value`)
  - Capture stdout/stderr: redirection via `rdbuf` (RAII with `CaptureGuard`) to capture the output of `cmd_*` without modification
  - Dispatch: mapping table `name` → `cmd_*` with special handling for `taskman_init` (direct call to `init_schema`)
  - JSON-RPC errors: -32700 (Parse error), -32600 (Invalid Request), -32601 (Method not found), -32602 (Invalid params / Unknown tool / Unsupported protocol version)
  - Business errors: returned in `result` with `isError: true` and message in `content[0].text`
- Tests: `tests/test_mcp.cpp` with 11 test cases covering initialize, tools/list, tools/call, ping, errors (unknown tool, invalid arguments, unknown method, parse error)
- Documentation: MCP section in USAGE.md (section 9) with Cursor configuration, examples, and tools description

## [0.11.1] - 2026-01-25

### Added

- **List view (web)**: next to Status, display "(blocked)" when the task depends on another non-`done` task, using `/task_deps` and `/task/:id` to get status of dependencies

## [0.11.0] - 2026-01-25

### Added

- **GET /milestone/:id**: JSON API for a milestone (404 if missing)
- **GET /milestones?limit=30&page=2**: JSON API listing milestones with pagination (limit 1–100, default 30; page ≥ 1)
- **GET /phase/:id**: JSON API for a phase (404 if missing)
- **GET /phases?limit=30&page=2**: JSON API listing phases with pagination (limit 1–100, default 30; page ≥ 1)
- **GET /task_deps**: JSON API listing task dependencies (task_id, depends_on), pagination limit 1–500 (default 100), page ≥ 1, optional filter `?task_id=`
- **GET /task/:id/deps**: JSON API for dependencies of a given task (404 if the task does not exist)

## [0.10.0] - 2026-01-25

### Added

- **taskman web** [--host <addr>] [--port <n>]: HTTP server for database consultation (default 127.0.0.1:8080)
- **GET /**: minimal HTML page including `style.css` and `main.js` (type=module)
- **GET /style.css**, **GET /main.js**: embedded stylesheet and script
- **GET /task/:id**: JSON API for a task (404 if missing)
- **GET /tasks?limit=20&page=1**: JSON API listing tasks with pagination (limit 1–100, page ≥ 1) and optional filters `phase`, `status`, `role`
- Dependency: **cpp-httplib** (FetchContent, v0.14.3); HTTPLIB_USE_*_IF_AVAILABLE=OFF options to avoid OpenSSL/ZLIB/Brotli/zstd

## [0.9.6] - 2026-01-25

### Added

- **TASKMAN_JOURNAL_MEMORY=1** and **CURSOR_AGENT**: when set, `PRAGMA journal_mode=MEMORY` is applied after opening the DB so SQLite does not create a `-journal` file on disk. Avoids "disk I/O error" in sandboxed environments (e.g. Cursor’s agent) where the journal file is blocked. Help (`taskman -h`) and docs/build.md document these variables.
- **docs/build.md**: "disk I/O error when using taskman from Cursor’s agent" section (Options A/B/C: env inline, agent rules, CURSOR_AGENT auto)
- **USAGE.md**: `TASKMAN_JOURNAL_MEMORY`, `CURSOR_AGENT` in the env table; tip for Cursor agent
- **Tests**: integration test with `TASKMAN_JOURNAL_MEMORY=1` (init, phase:add, phase:list)

## [0.9.5] - 2026-01-25

### Added

- **taskman <command> --help** (or **-h**): detailed per-command help. Commands using cxxopts (phase:add/edit, milestone:add/edit/list, task:add/get/list/edit, task:dep:add/remove) show options via `opts.help()`. `init` and `phase:list` have dedicated help strings. The main help (`taskman -h`) now includes: *Use 'taskman <command> --help' for command-specific options.*

## [0.9.4] - 2026-01-25

### Added

- **scripts/create_demo.py**: Python script that creates a populated demo database (phases, milestones, tasks, dependencies, sample statuses) by invoking the taskman CLI. Arguments: taskman executable path (required), DB path (optional, default `./demo.db`). Overwrites the DB if it exists; removes `-journal`, `-wal`, `-shm` sidecars. Demo scenario: e-commerce site MVP (Design, Development, Acceptance, Delivery).

### Changed

- **print_usage()**: command list is now built from a `CmdInfo[]` table and printed in a loop; adding a command only requires one new table entry
- **Version**: `TASKMAN_VERSION` is generated at configure time from the `VERSION` file via `version.h`; `project(taskman VERSION …)` and `version.h` both use `VERSION` as the single source of truth

## [0.9.3] - 2026-01-25

### Added

- `-v`, `--version`: print version and exit 0

## [0.9.2] - 2026-01-25

### Added

- docs/changelog_user.md: user-facing changelog (commands, options, features; no implementation details)

### Changed

- README.md: shortened description; links grouped by audience (Users: USAGE, CHANGELOG_USER; Developers: BUILD, CHANGELOG)
- USAGE.md: "See also" section points to CHANGELOG_USER for users, BUILD/CHANGELOG for developers

## [0.9.1] - 2026-01-25

### Added

- USAGE.md: user guide

## [0.9.0] — 2026-01-25

### Added

- Phase 8 — Quality and Deployment:
  - **Integration tests** (`tests/test_integration.cpp`): runs `taskman` as a subprocess, uses temporary DB fixtures; tests include: init then phase:list (empty), init + phase:add + phase:list, init + phase:add + task:add + task:get, unknown command → exit 1; SKIP if executable not found; uses `add_dependencies(tests taskman)` to enforce build order
  - **GitHub Actions** (`.github/workflows/build.yml`): Release build workflow for Windows (MSVC), Linux (GCC, Clang), and macOS (Clang); binary artifacts are `taskman-linux-gcc`, `taskman-linux-clang`, `taskman-macos-clang`, `taskman-windows-msvc`; triggered on push/PR to main or master

## [0.8.0] — 2026-01-25

### Added

- Phase 7 — Output formats:
  - **JSON**: `formats` module (phase_to_json, milestone_to_json, task_to_json); consistent structures for phase, milestone, task; absent optional fields → `null`; `sort_order` and `reached` as integers
  - **Text**: `print_task_text` for task:add, task:get, task:list; order: title, description, status, role, then id, phase_id, milestone_id, sort_order
  - **Exit codes**: 0 success, 1 error (parsing, DB, argument validation), documented in main.cpp
- Tests: cmd_task_add --format text, cmd_task_get --format text, cmd_task_list --format text

## [0.7.0] — 2026-01-25

### Added

- Phase 6 — Task dependencies:
  - `task:dep:add`: <task-id> <dep-id> → INSERT into task_deps; existence check for both tasks; reject self-dependency and duplicates
  - `task:dep:remove`: <task-id> <dep-id> → DELETE
- Unit tests: task:dep:add (success, non-existent task, self-dependency, duplicate), task:dep:remove (success, idempotent)

## [0.6.0] — 2026-01-25

### Added

- Phase 5 — Tasks:
  - `task:add`: --title, --phase, [--description], [--role], [--milestone], [--format json|text]; UUID v4 ID; INSERT; output = created task (JSON or text)
  - `task:get`: <id> [--format json|text]; JSON by default
  - `task:list`: [--phase], [--status], [--role], [--format json|text]; filtered SELECT; JSON or text output
  - `task:edit`: <id> [--title] [--description] [--status] [--role] [--milestone] → partial UPDATE
- JSON and text output for task (fields id, phase_id, milestone_id, title, description, status, sort_order, role)
- Validation --role, --status (to_do, in_progress, done; project-manager, project-designer, software-architect, developer, summary-writer, documentation-writer)
- Unit tests: `test_task.cpp` (task:add with UUID v4, task:get, task:list, task:edit, validations)

## [0.5.0] — 2026-01-25

### Added

- Phase 4 — Milestones:
  - `milestone:add`: --id, --phase, [--name], [--criterion], [--reached 0|1] → INSERT
  - `milestone:edit`: <id> [--name] [--criterion] [--reached 0|1] [--phase <id>] → partial UPDATE
  - `milestone:list`: [--phase <id>] → SELECT (filtered if --phase); JSON output (id, phase_id, name, criterion, reached)
- DB layer: `Database::query(sql, params)` (parameterized SELECT); `sqlite3_open_v2` + `PRAGMA busy_timeout=3000`; error log if `sqlite3_close` fails
- Unit tests: `test_milestone.cpp` (milestone:add, milestone:edit, milestone:list); `Database::query` with parameters in `test_db.cpp`
- docs/build.md: Troubleshooting section (`.db-journal` file, "disk I/O error")

## [0.4.0] — 2026-01-25

### Added

- Unit tests (Catch2):
  - DB layer: `Database::open`, `close`, `exec`, `run`, `query`, `init_schema` (no connection, parameters, NULL, SELECT)
  - Phase commands: `cmd_phase_add`, `cmd_phase_edit`, `cmd_phase_list` (success, --id/--name/--status/--sort-order validation, JSON formatting including `sort_order` as number)

## [0.3.0] — 2026-01-25

### Added

- Phase 3 — Phases:
  - `phase:add`: --id, --name, [--status to_do|in_progress|done], [--sort-order] → INSERT
  - `phase:edit`: <id> [--name] [--status] [--sort-order] → partial UPDATE
  - `phase:list`: SELECT * FROM phases ORDER BY sort_order → JSON (array of objects id, name, status, sort_order)
  - DB layer: `Database::run()` (parameterized queries), `Database::query()` (SELECT → rows)

## [0.2.0] — 2026-01-25

### Added

- Phase 2 — Database:
  - DB layer (`src/db.hpp`, `src/db.cpp`): `Database::open`, `Database::exec`, `init_schema`
  - DB path: `TASKMAN_DB_NAME` (env), default `project_tasks.db`; open/create
  - `taskman init`: runs the 4 `CREATE TABLE IF NOT EXISTS` (phases, milestones, tasks, task_deps)
  - Helpers: SQLite error handling → stderr + exit code 1

## [0.1.0] — 2026-01-25

### Added

- Phase 1 — Project setup:
  - Structure `src/`, `third_party/sqlite/`
  - CMakeLists.txt: C++17, `taskman` executable
  - SQLite: amalgamation via FetchContent (azadkuh/sqlite-amalgamation)
  - nlohmann/json, cxxopts, stduuid: FetchContent
  - `TASKMAN_DB_NAME` (env), default `project_tasks.db`
  - Entry point `main.cpp`: dispatch argc/argv to subcommands (stubs)
