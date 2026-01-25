# Changelog

## [0.9.2] - 2026-01-25

### Added

- CHANGELOG_USER.md: user-facing changelog (commands, options, features; no implementation details)

### Changed

- README.md: shortened description; links grouped by audience (Users: USAGE, CHANGELOG_USER; Developers: BUILD, CHANGELOG)
- USAGE.md: "See also" section points to CHANGELOG_USER for users, BUILD/CHANGELOG for developers

## [0.9.1] - 2026-01-25

### Added

- USAGE.md : user guide

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
  - `task:dep:add`: \<task-id\> \<dep-id\> → INSERT into task_deps; existence check for both tasks; reject self-dependency and duplicates
  - `task:dep:remove`: \<task-id\> \<dep-id\> → DELETE
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
- BUILD.md: Troubleshooting section (`.db-journal` file, "disk I/O error")

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
