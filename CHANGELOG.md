# Changelog

## [0.6.0] — 2025-01-25

### Ajouté

- Phase 5 — Tasks :
  - `task:add` : --title, --phase, [--description], [--role], [--milestone], [--format json|text] ; ID UUID v4 ; INSERT ; sortie = tâche créée (JSON ou text)
  - `task:get` : <id> [--format json|text] ; JSON par défaut
  - `task:list` : [--phase], [--status], [--role], [--format json|text] ; SELECT filtré ; sortie JSON ou text
  - `task:edit` : <id> [--title] [--description] [--status] [--role] [--milestone] → UPDATE partiel
- Sorties JSON et text pour task (champs id, phase_id, milestone_id, title, description, status, sort_order, role)
- Validation --role, --status (to_do, in_progress, done ; project-manager, project-designer, software-architect, developer, summary-writer, documentation-writer)
- Tests unitaires : `test_task.cpp` (task:add avec UUID v4, task:get, task:list, task:edit, validations)

## [0.5.0] — 2025-01-25

### Ajouté

- Phase 4 — Milestones :
  - `milestone:add` : --id, --phase, [--name], [--criterion], [--reached 0|1] → INSERT
  - `milestone:edit` : <id> [--name] [--criterion] [--reached 0|1] [--phase <id>] → UPDATE partiel
  - `milestone:list` : [--phase <id>] → SELECT (filtré si --phase) ; sortie JSON (id, phase_id, name, criterion, reached)
- Couche DB : `Database::query(sql, params)` (SELECT paramétré) ; `sqlite3_open_v2` + `PRAGMA busy_timeout=3000` ; log d’erreur si `sqlite3_close` échoue
- Tests unitaires : `test_milestone.cpp` (milestone:add, milestone:edit, milestone:list) ; `Database::query` avec paramètres dans `test_db.cpp`
- BUILD.md : section Dépannage (fichier `-journal`, erreur « disk I/O error »)

## [0.4.0] — 2025-01-25

### Ajouté

- Tests unitaires (Catch2) :
  - Couche DB : `Database::open`, `close`, `exec`, `run`, `query`, `init_schema` (sans connexion, paramètres, NULL, SELECT)
  - Commandes phase : `cmd_phase_add`, `cmd_phase_edit`, `cmd_phase_list` (succès, validation --id/--name/--status/--sort-order, formatage JSON dont `sort_order` en nombre)

## [0.3.0] — 2025-01-25

### Ajouté

- Phase 3 — Phases :
  - `phase:add` : --id, --name, [--status to_do|in_progress|done], [--sort-order] → INSERT
  - `phase:edit` : <id> [--name] [--status] [--sort-order] → UPDATE partiel
  - `phase:list` : SELECT * FROM phases ORDER BY sort_order → JSON (tableau d’objets id, name, status, sort_order)
  - Couche DB : `Database::run()` (requêtes paramétrées), `Database::query()` (SELECT → lignes)

## [0.2.0] — 2025-01-25

### Ajouté

- Phase 2 — Base de données :
  - Couche DB (`src/db.hpp`, `src/db.cpp`) : `Database::open`, `Database::exec`, `init_schema`
  - Chemin base : `TASKMAN_DB_NAME` (env), défaut `project_tasks.db` ; open/create
  - `taskman init` : exécute les 4 `CREATE TABLE IF NOT EXISTS` (phases, milestones, tasks, task_deps)
  - Helpers : gestion erreurs SQLite → stderr + code de sortie 1

## [0.1.0] — 2025-01-25

### Ajouté

- Phase 1 — Setup projet :
  - Structure `src/`, `third_party/sqlite/`
  - CMakeLists.txt : C++17, exécutable `taskman`
  - SQLite : amalgamation via FetchContent (azadkuh/sqlite-amalgamation)
  - nlohmann/json, cxxopts, stduuid : FetchContent
  - `TASKMAN_DB_NAME` (env), défaut `project_tasks.db`
  - Point d’entrée `main.cpp` : dispatch argc/argv vers sous-commandes (stubs)
