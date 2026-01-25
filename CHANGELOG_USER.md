# Changelog (users)

User-facing changes: new commands, options, formats, and behavior.

## [0.12.0] - 2026-01-25

- **taskman mcp** : mode serveur MCP (Model Context Protocol) sur stdio. Lit les requêtes JSON-RPC sur stdin, écrit les réponses sur stdout. À configurer comme serveur MCP dans Cursor (commande `taskman`, args `mcp`). La phase « Infra et squelette » est en place ; les méthodes `initialize`, `tools/list`, `tools/call` seront ajoutées dans les phases suivantes.

## [0.11.1] - 2026-01-25

- **Vue liste (web)** : à côté du Statut, affichage de « (blocked) » si la tâche dépend d’une tâche non-`done` (utilisation de `/task_deps` et `/task/:id`).

## [0.11.0] - 2026-01-25

- **API web** : GET /milestone/:id, GET /milestones (limit, page), GET /phase/:id, GET /phases (limit, page), GET /task_deps (limit, page, ?task_id=), GET /task/:id/deps.

## [0.10.0] - 2026-01-25

- **taskman web** [--host \<addr\>] [--port \<n\>] : démarre un serveur HTTP (défaut http://127.0.0.1:8080). La page d’accueil charge une interface qui consulte les tâches via l’API JSON (GET /tasks, GET /task/\<id\>).

## [0.9.6] - 2026-01-25

- **Cursor agent / "disk I/O error"**: set `TASKMAN_JOURNAL_MEMORY=1` when running taskman from Cursor’s agent (or ensure `CURSOR_AGENT` is set) to avoid SQLite "disk I/O error" in sandboxed environments. See [BUILD.md](BUILD.md).

## [0.9.5] - 2026-01-25

- **taskman \<command\> --help** (or **-h**): detailed help for each command (options, required/optional args). The main help (`taskman -h`) indicates: *Use 'taskman \<command\> --help' for command-specific options.*

## [0.9.3] - 2026-01-25

- **Options**: `-v`, `--version` — show version and exit.

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
