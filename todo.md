# Todo — taskman

## 1. Setup projet

- [x] Créer structure : `src/`, `third_party/` (ou `external/`)
- [x] CMakeLists.txt : C++17, type `executable` unique
- [x] Intégrer SQLite : amalgamation (sqlite3.c, sqlite3.h) dans `third_party/sqlite/` (FetchContent: azadkuh/sqlite-amalgamation)
- [x] Intégrer nlohmann/json (json.hpp) : FetchContent ou copie dans `third_party/`
- [x] Intégrer cxxopts : FetchContent ou copie dans `third_party/`
- [x] Intégrer lib UUID v4 : header-only (ex. [stduuid](https://github.com/mariusbancila/stduuid)) ou source vendorable
- [x] Lire `TASKMAN_DB_NAME` (env) ; défaut `project_tasks.db`
- [x] Point d’entrée `main.cpp` : dispatch `argc/argv` vers sous-commandes

## 2. Base de données

- [x] Couche DB : init connexion, chemin `TASKMAN_DB_NAME`, open/create
- [x] `taskman init` : exécuter les 4 `CREATE TABLE IF NOT EXISTS` (phases, milestones, tasks, task_deps)
- [x] Helpers : exécution requêtes, gestion erreurs SQLite → code sortie 1 + stderr

## 3. Phases

- [x] `phase:add` : --id, --name, [--status], [--sort-order] → INSERT
- [x] `phase:edit` : <id> + champs optionnels → UPDATE partiel
- [x] `phase:list` : SELECT * ORDER BY sort_order ; sortie JSON (structure à définir : tableau d’objets)

## 4. Milestones

- [x] `milestone:add` : --id, --phase, [--name], [--criterion], [--reached] → INSERT
- [x] `milestone:edit` : <id> + champs optionnels (dont --phase) → UPDATE
- [x] `milestone:list` : [--phase <id>] → SELECT (filtré si --phase) ; sortie JSON

## 5. Tasks

- [x] `task:add` : --title, --phase, [--description], [--role], [--milestone] ; génération ID (UUID v4) ; INSERT ; sortie = tâche créée (comme `task:get`)
- [x] `task:get` : <id> ; [--format json|text] ; JSON par défaut
- [x] `task:list` : [--phase], [--status], [--role] ; [--format json|text]
- [x] `task:edit` : <id> + champs optionnels → UPDATE partiel

## 6. Dépendances entre tâches

- [x] `task:dep:add` : <task-id> <dep-id> → INSERT dans task_deps ; vérifier existence des tâches
- [x] `task:dep:remove` : <task-id> <dep-id> → DELETE

## 7. Formats de sortie

- [x] JSON : nlohmann/json ; structures pour phase, milestone, task (avec champs optionnels omis ou null selon spec)
- [x] Text : format lisible (ex. titre, description, status, role sur lignes distinctes) pour task:add, task:get, task:list
- [x] Codes de sortie : 0 succès, 1 erreur (conventions pour parsing, DB, validation des args)

## 8. Qualité et déploiement

- [x] Tests : unitaires (helpers DB, formatage, milestone:add/edit/list) ; [x] unitaires génération ID (avec task:add) ; [x] intégration (appels `taskman` en subprocess, fixtures DB)
- [x] GitHub Actions : workflow build (Release) pour Windows (MSVC ou MinGW), Linux (GCC/Clang), macOS (Clang) ; artefacts binaires
- [x] Changelog et version (package.json ou VERSION / CHANGELOG.md) alignés avec les livrables

---

*Référence : `specs.md`*
