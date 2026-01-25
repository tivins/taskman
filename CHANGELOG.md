# Changelog

## [0.1.0] — 2025-01-25

### Ajouté

- Phase 1 — Setup projet :
  - Structure `src/`, `third_party/sqlite/`
  - CMakeLists.txt : C++17, exécutable `taskman`
  - SQLite : amalgamation via FetchContent (azadkuh/sqlite-amalgamation)
  - nlohmann/json, cxxopts, stduuid : FetchContent
  - `TASKMAN_DB_NAME` (env), défaut `project_tasks.db`
  - Point d’entrée `main.cpp` : dispatch argc/argv vers sous-commandes (stubs)
