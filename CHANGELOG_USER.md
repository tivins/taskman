# Changelog (users)

User-facing changes: new commands, options, formats, and behavior.

## [0.9.2] - 2026-01-25

- This user changelog; README reorganized so Users and Developers find their docs quickly.

## [0.9.1] - 2026-01-25

- User guide: [USAGE.md](USAGE.md).

## [0.9.0] - 2026-01-25

- GitHub Actions CI: release artifacts for Linux, macOS, Windows. No impact on CLI usage.

## [0.8.0] - 2026-01-25

- **Formats**: JSON for phases, milestones, tasks; `--format text` for `task:add`, `task:get`, `task:list`.
- **Exit codes**: 0 = success, 1 = error (parsing, database, validation).

## [0.7.0] - 2026-01-25

- `task:dep:add <task-id> <dep-id>` and `task:dep:remove <task-id> <dep-id>`. Both tasks must exist; no self-dependency or duplicates.

## [0.6.0] - 2026-01-25

- **Tasks**: `task:add` (--title, --phase, --description, --role, --milestone, --format), `task:get`, `task:list` (--phase, --status, --role, --format), `task:edit`. Task ID is UUID v4.
- **Roles**: project-manager, project-designer, software-architect, developer, summary-writer, documentation-writer.
- **Statuses**: to_do, in_progress, done.

## [0.5.0] - 2026-01-25

- **Milestones**: `milestone:add` (--id, --phase, --name, --criterion, --reached), `milestone:edit`, `milestone:list [--phase]`.
- BUILD.md: troubleshooting for "disk I/O error" and `.db-journal` files.

## [0.3.0] - 2026-01-25

- **Phases**: `phase:add` (--id, --name, --status, --sort-order), `phase:edit`, `phase:list` (JSON).

## [0.2.0] - 2026-01-25

- `taskman init` to create the database. Environment variable `TASKMAN_DB_NAME` (default `project_tasks.db`).
