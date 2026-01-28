# Changelog

## [0.20.0] - 2026-01-28

### Changed

- **Réorganisation de la structure du projet** : déplacement des fichiers intégrés dans le binaire vers un dossier dédié `embed/` pour permettre l'utilisation de namespaces dans `src/` :
  - `src/web/` → `embed/web/` (assets web : CSS et fichiers JavaScript)
  - `docs/roles_agents/` → `embed/roles_agents/` (fichiers markdown des agents)
- Mise à jour de `CMakeLists.txt`, `scripts/embed_assets.py`, `scripts/embed_agents.py`, `src/web.cpp` et `docs/USAGE_WEB.md` pour utiliser les nouveaux chemins.

## [0.19.0] - 2026-01-28

### Changed

- **Refactoring SOLID - Classe Database** : séparation des responsabilités selon le principe SRP (Single Responsibility Principle). La classe `Database` a été refactorisée en trois classes spécialisées :
  - `DatabaseConnection` : gestion exclusive de la connexion SQLite (open, close, is_open)
  - `QueryExecutor` : exécution exclusive des requêtes SQL (exec, run, query)
  - `SchemaManager` : gestion exclusive du schéma et des migrations (init_schema, table_has_column, ensure_timestamps)
- La classe `Database` devient une façade qui délègue aux trois classes spécialisées, maintenant la compatibilité avec le code existant tout en respectant les principes SOLID.

### Added

- **Nouveaux fichiers** : `db_connection.hpp`/`db_connection.cpp`, `query_executor.hpp`/`query_executor.cpp`, `schema_manager.hpp`/`schema_manager.cpp`

## [0.18.1] - 2026-01-28

### Added

- **Vue web** : route GET `/task/:id/notes` et section « Historique des notes » dans la vue détail d’une tâche, pour consulter les notes ajoutées par l’agent pendant la réalisation (id, content, kind, role, created_at).

## [0.18.0] - 2026-01-27

### Added

- **Notes sur les tâches** : table `task_notes` (id, task_id, content, kind, role, created_at), créée par `init_schema`. Fonction réutilisable `note_add()` et commandes CLI `task:note:add <task-id> --content "..." [--kind completion|progress|issue] [--role <role>] [--format json|text]` et `task:note:list <task-id> [--format json|text]`. Outils MCP `taskman_task_note_add` et `taskman_task_note_list`. La tâche doit exister ; `kind` et `role` sont optionnels. Sortie : note créée (add) ou liste ordonnée par `created_at` (list).
- **demo:generate** : ajout de 7 notes (completion / progress) sur des tâches de la démo (t1, t2, t3, t4, t5, t7, t15).

### Changed

- **note_add** : à l'ajout d'une note, mise à jour de `updated_at` sur la tâche liée (`UPDATE tasks SET updated_at = datetime('now') WHERE id = ?`).

## [0.17.1] - 2026-01-27

### Fixed

- **Build (MSVC)** : `scripts/embed_assets.py` découpe désormais les littéraux de chaîne (CSS/JS) en blocs d’au plus 16 000 caractères pour rester sous la limite MSVC (~16 380, erreur C2026). Les blocs sont émis comme littéraux concaténés. Corrige la compilation sur Windows avec MSVC.

## [0.17.0] - 2026-01-27

### Added

- **created_at, updated_at** sur les tables `phases`, `milestones`, `tasks` : colonnes automatiques (non modifiables par l’utilisateur). `created_at` et `updated_at` sont définies à l’insertion via `DEFAULT (datetime('now'))`. À chaque mise à jour (phase:edit, milestone:edit, task:edit), `updated_at` est rafraîchi. Migration automatique pour les bases existantes (ajout des colonnes si absentes). Ces champs sont exposés en JSON et en format text sur tous les canaux (CLI, WEB, MCP).

### Changed

- **Schéma** : `init_schema` crée les tables avec `created_at` et `updated_at` ; une migration ajoute ces colonnes aux tables existantes via `pragma_table_info` et `ALTER TABLE ... ADD COLUMN`.
- **Formats** : `phase_to_json`, `milestone_to_json`, `task_to_json` et `print_task_text` incluent `created_at` et `updated_at`.

## [0.16.0] - 2026-01-27

### Changed

- **Refactorisation utilitaires phase/milestone/task** : les commandes `phase:add`, `milestone:add`, `task:add`, `task:dep:add` et `demo:generate` s'appuient désormais sur des fonctions utilitaires réutilisables (`phase_add`, `milestone_add`, `task_add`, `task_dep_add`) déclarées dans les headers. Cela évite la duplication de logique SQL et de validation, centralise les règles (status, reached, roles) et permet un usage programmatique sans construire artificiellement des `argv`. Voir `docs/REFACTORING.md`.
- **demo:generate** utilise `phase_add`, `milestone_add`, `task_add` et `task_dep_add` au lieu de requêtes SQL directes.

### Added

- **phase_add(Database&, id, name, status?, sort_order?)** dans `phase.hpp` : ajout d'une phase avec validation du status.
- **milestone_add(Database&, id, phase_id, name?, criterion?, reached)** dans `milestone.hpp` : ajout d'un milestone.
- **task_add(Database&, id, phase_id, milestone_id?, title, description?, status?, sort_order?, role?)** dans `task.hpp` : ajout d'une tâche avec validation status et role.
- **task_dep_add(Database&, task_id, depends_on)** dans `task.hpp` : ajout d'une dépendance entre tâches (vérification existence et non auto-dépendance).
- Tests unitaires directs pour ces fonctions dans `tests/test_phase.cpp`, `tests/test_milestone.cpp`, `tests/test_task.cpp`.

## [0.15.0] - 2026-01-27

### Added

- **taskman demo:generate**: commande intégrée pour créer une base de données de démonstration remplie avec un exemple réaliste (projet MVP de site e-commerce). Remplace le script Python `scripts/create_demo.py` et permet d'utiliser directement les fonctions du codebase C++ sans avoir à fournir le chemin de l'exécutable. La commande supprime automatiquement la base de données existante (et ses fichiers journal) avant de créer la nouvelle, garantissant un résultat reproductible. Crée 4 phases (Design, Development, Acceptance, Delivery), 4 milestones, 9 tâches avec priorités (sort_order), et leurs dépendances.
- **src/demo.cpp**, **src/demo.hpp**: module implémentant la commande `demo:generate` avec création directe des données via SQL.

## [0.14.0] - 2026-01-27

### Added

- **taskman mcp:config** [--executable <path>] [--output <file>]: génère ou met à jour le fichier `.cursor/mcp.json` avec la configuration du serveur MCP taskman. Prend en paramètre le chemin absolu de l'exécutable taskman (ou taskman.exe). Pour `TASKMAN_DB_NAME`, utilise le répertoire courant + "project_tasks.db". Fusionne avec les serveurs MCP existants dans le fichier. Similaire à `agents:generate` pour la configuration MCP.
- **src/mcp_config.cpp**, **src/mcp_config.hpp**: module implémentant la commande `mcp:config` avec gestion des fichiers JSON (lecture, fusion, écriture).

## [0.13.0] - 2026-01-27

### Added

- **taskman agents:generate** [--output <dir>]: génère les fichiers agents dans `.cursor/agents/` (ou le répertoire spécifié) à partir des fichiers markdown intégrés dans le binaire. Les fichiers sources sont dans `docs/roles_agents/*.md` et sont intégrés dans le binaire lors de la compilation via `scripts/embed_agents.py`. La commande extrait ces fichiers et les écrit dans le répertoire de sortie.
- **scripts/embed_agents.py**: script Python qui génère `agents.generated.h` à partir des fichiers markdown dans `docs/roles_agents/`. Les fichiers sont intégrés comme raw string literals dans le header généré.
- **src/agents.cpp**, **src/agents.hpp**: module implémentant la commande `agents:generate` avec gestion des fichiers système (création de répertoires, écriture de fichiers).
- **CMakeLists.txt**: ajout de la génération de `agents.generated.h` via `add_custom_command` avec dépendances sur tous les fichiers `.md` dans `docs/roles_agents/`.

## [0.12.0] - 2026-01-25

### Added

- **taskman mcp** : mode serveur MCP (Model Context Protocol) sur stdio. Lit les requêtes JSON-RPC ligne par ligne sur stdin, écrit les réponses sur stdout. Implémentation complète du protocole MCP 2025-11-25 :
  - Lifecycle : `initialize` (avec validation de `protocolVersion`), `notifications/initialized`
  - Méthodes : `tools/list` (retourne les 13 outils), `tools/call` (exécute les commandes CLI), `ping`
  - Table des 13 outils : définitions complètes avec `name`, `description`, `inputSchema` (JSON Schema), `positional_keys`
  - Conversion `arguments` → `argv` : construction automatique des arguments CLI à partir des paramètres JSON (positionnels puis options `--key value`)
  - Capture stdout/stderr : redirection via `rdbuf` (RAII avec `CaptureGuard`) pour capturer la sortie des `cmd_*` sans modification
  - Dispatch : table de correspondance `name` → `cmd_*` avec gestion spéciale pour `taskman_init` (appel direct à `init_schema`)
  - Erreurs JSON-RPC : -32700 (Parse error), -32600 (Invalid Request), -32601 (Method not found), -32602 (Invalid params / Unknown tool / Unsupported protocol version)
  - Erreurs métier : retournées dans `result` avec `isError: true` et message dans `content[0].text`
- Tests : `tests/test_mcp.cpp` avec 11 cas de test couvrant initialize, tools/list, tools/call, ping, erreurs (outil inconnu, arguments invalides, méthode inconnue, parse error)
- Documentation : section MCP dans USAGE.md (section 9) avec configuration Cursor, exemples, et description des outils

## [0.11.1] - 2026-01-25

### Added

- **Vue liste (web)** : à côté du Statut, affichage de « (blocked) » lorsque la tâche dépend d’une tâche non-`done`, en s’appuyant sur `/task_deps` et `/task/:id` pour les statuts des `depends_on`.

## [0.11.0] - 2026-01-25

### Added

- **GET /milestone/:id** : API JSON d'un jalon (404 si absent).
- **GET /milestones?limit=30&page=2** : API JSON listant les jalons avec pagination (limit 1–100, défaut 30 ; page ≥ 1).
- **GET /phase/:id** : API JSON d'une phase (404 si absente).
- **GET /phases?limit=30&page=2** : API JSON listant les phases avec pagination (limit 1–100, défaut 30 ; page ≥ 1).
- **GET /task_deps** : API JSON listant les dépendances entre tâches (task_id, depends_on), pagination limit 1–500 (défaut 100), page ≥ 1, filtre optionnel `?task_id=`.
- **GET /task/:id/deps** : API JSON des dépendances d’une tâche donnée (404 si la tâche n’existe pas).

## [0.10.0] - 2026-01-25

### Added

- **taskman web** [--host \<addr\>] [--port \<n\>]: serveur HTTP pour consultation de la base (défaut 127.0.0.1:8080).
- **GET /** : page HTML minimale avec inclusion de `style.css` et `main.js` (type=module).
- **GET /style.css**, **GET /main.js** : feuille de style et script embarqués.
- **GET /task/:id** : API JSON d’une tâche (404 si absente).
- **GET /tasks?limit=20&page=1** : API JSON listant les tâches avec pagination (limit 1–100, page ≥ 1) et filtres optionnels `phase`, `status`, `role`.
- Dépendance **cpp-httplib** (FetchContent, v0.14.3) ; options HTTPLIB_USE_*_IF_AVAILABLE=OFF pour éviter OpenSSL/ZLIB/Brotli/zstd.

## [0.9.6] - 2026-01-25

### Added

- **TASKMAN_JOURNAL_MEMORY=1** and **CURSOR_AGENT**: when set, `PRAGMA journal_mode=MEMORY` is applied after opening the DB so SQLite does not create a `-journal` file on disk. Avoids "disk I/O error" in sandboxed environments (e.g. Cursor’s agent) where the journal file is blocked. Help (`taskman -h`) and docs/BUILD.md document these variables.
- **docs/BUILD.md**: "disk I/O error when using taskman from Cursor’s agent" section (Options A/B/C: env inline, agent rules, CURSOR_AGENT auto).
- **USAGE.md**: `TASKMAN_JOURNAL_MEMORY`, `CURSOR_AGENT` in the env table; tip for Cursor agent.
- **Tests**: integration test with `TASKMAN_JOURNAL_MEMORY=1` (init, phase:add, phase:list).

## [0.9.5] - 2026-01-25

### Added

- **taskman \<command\> --help** (or **-h**): detailed per-command help. Commands using cxxopts (phase:add/edit, milestone:add/edit/list, task:add/get/list/edit, task:dep:add/remove) show options via `opts.help()`. `init` and `phase:list` have dedicated help strings. The main help (`taskman -h`) now includes: *Use 'taskman \<command\> --help' for command-specific options.*

## [0.9.4] - 2026-01-25

### Added

- **scripts/create_demo.py**: Python script that creates a populated demo database (phases, milestones, tasks, dependencies, sample statuses) by invoking the taskman CLI. Arguments: taskman executable path (required), DB path (optional, default `./demo.db`). Overwrites the DB if it exists; removes `-journal`, `-wal`, `-shm` sidecars. Demo scenario: e‑commerce site MVP (Design, Development, Acceptance, Delivery).

### Changed

- **print_usage()**: command list is now built from a `CmdInfo[]` table and printed in a loop; adding a command only requires one new table entry.
- **Version**: `TASKMAN_VERSION` is generated at configure time from the `VERSION` file via `version.h`; `project(taskman VERSION …)` and `version.h` both use `VERSION` as the single source of truth.

## [0.9.3] - 2026-01-25

### Added

- `-v`, `--version`: print version and exit 0

## [0.9.2] - 2026-01-25

### Added

- docs/CHANGELOG_USER.md: user-facing changelog (commands, options, features; no implementation details)

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
- docs/docs/BUILD.md: Troubleshooting section (`.db-journal` file, "disk I/O error")

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
