# Changelog

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
