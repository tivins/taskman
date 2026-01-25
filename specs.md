# Project manager | tasks manager

## But

`taskman` est un exécutable (fichier unique) utilisable uniquement en [CLI](#cli).  
Les agents **project-manager** créent et modifient les phases et milestones ; les autres agents récupèrent et modifient les tâches.

## Structure de la base de données

```sqlite
CREATE TABLE IF NOT EXISTS phases (
  id TEXT PRIMARY KEY,
  name TEXT NOT NULL,
  status TEXT DEFAULT 'to_do',
  sort_order INTEGER
);

CREATE TABLE IF NOT EXISTS milestones (
  id TEXT PRIMARY KEY,
  phase_id TEXT NOT NULL,
  name TEXT,
  criterion TEXT,
  reached INTEGER DEFAULT 0,
  FOREIGN KEY (phase_id) REFERENCES phases(id)
);

CREATE TABLE IF NOT EXISTS tasks (
  id TEXT PRIMARY KEY,
  phase_id TEXT NOT NULL,
  milestone_id TEXT,
  title TEXT,
  description TEXT,
  status TEXT DEFAULT 'to_do',
  sort_order INTEGER,
  role TEXT,
  FOREIGN KEY (phase_id) REFERENCES phases(id),
  FOREIGN KEY (milestone_id) REFERENCES milestones(id)
);

CREATE TABLE IF NOT EXISTS task_deps (
  task_id TEXT NOT NULL,
  depends_on TEXT NOT NULL,
  PRIMARY KEY (task_id, depends_on),
  FOREIGN KEY (task_id) REFERENCES tasks(id),
  FOREIGN KEY (depends_on) REFERENCES tasks(id)
);
```

## Enumerations

- **role** : project-manager, project-designer, software-architect, developer, summary-writer, documentation-writer
- **status** (phases, tasks) : to_do, in_progress, done
- **format** (task:add, task:get, task:list) : json, text
- **reached** (milestones) : 0, 1

## CLI

`taskman init` crée les tables SQLite ; fichier `project_tasks.db` par défaut (env. `TASKMAN_DB_NAME`).  
`phase:edit`, `milestone:edit` et `task:edit` ne modifient que les champs fournis. Statuts : `to_do`, `in_progress`, `done`. Sortie JSON par défaut pour `task:add`, `task:get` et `task:list`, `--format text` en option. `task:add` renvoie la tâche créée au même format que `task:get`. Codes de sortie : 0 succès, 1 erreur.

```shell
taskman init

taskman phase:add --id <id> --name "..." [--status to_do|in_progress|done] [--sort-order <n>]
taskman phase:edit <id> [--name "..."] [--status ...] [--sort-order <n>]
taskman phase:list

taskman milestone:add --id <id> --phase <id> [--name "..."] [--criterion "..."] [--reached 0|1]
taskman milestone:edit <id> [--name "..."] [--criterion "..."] [--reached 0|1] [--phase <id>]
taskman milestone:list [--phase <id>]

taskman task:add --title "..." --phase <id> [--description "..."] [--role "..."] [--milestone <id>] [--format json|text]
taskman task:edit <id> [--title "..."] [--description "..."] [--status to_do|in_progress|done] [--role "..."] [--milestone <id>]
taskman task:get <id> [--format json|text]
taskman task:list [--phase <id>] [--status <s>] [--role <r>] [--format json|text]

taskman task:dep:add <task-id> <dep-id>
taskman task:dep:remove <task-id> <dep-id>
```

## Exemple

```shell
# Project-manager : init et structure
taskman init
taskman phase:add --id p1 --name "Conception" --sort-order 1
taskman phase:add --id p2 --name "Développement" --sort-order 2
taskman milestone:add --id m1 --phase p1 --name "Spec validée" --criterion "Document de spec approuvé"
taskman milestone:add --id m2 --phase p2 --name "API livrée" --criterion "Endpoints documentés et testés"

# Ajout des tâches (ids t1, t2, t3 fournis par task:add ou task:list)
taskman task:add --title "Rédiger la spec API" --phase p1 --milestone m1 --role project-designer
taskman task:add --title "Définir le schéma de données" --phase p1 --milestone m1 --role software-architect
taskman task:add --title "Implémenter les endpoints" --phase p2 --milestone m2 --role developer
taskman task:dep:add t3 t2

# Agent : récupère une tâche, la traite, la marque faite
taskman task:list --phase p1
taskman task:get t1 --format text
taskman task:edit t1 --status in_progress
# ... travail ...
taskman task:edit t1 --status done
taskman milestone:edit m1 --reached 1
```

## Implémentation

### Contraintes

* OS : Windows/Linux/macOS
* déploiement : binaire distribué (Release sur Github)
* autonomie : binaire sans runtime
* langage : C++

### Organisation

* code : `/src` ; build : CMake

### Dépendances (intégrées, pas de runtime externe)

* **SQLite** : [amalgamation](https://sqlite.org/amalgamation.html) (sqlite3.c + sqlite3.h) — 1 fichier C à compiler. Wrapper C++ header-only (ex. [sqlitepp](https://github.com/nicb1/sqlitepp) ou [sqlite-hpp](https://github.com/elelel/sqlite-hpp)).
* **JSON** : [nlohmann/json](https://github.com/nlohmann/json) (json.hpp, header-only).
* **CLI** : parsing manuel `argc/argv` ou lib légère (ex. [cxxopts](https://github.com/jarro2783/cxxopts), header-only).

### IDs des tâches

* `task:add` : génération d’un id unique (UUID, slug du titre, ou compteur). À préciser à l’implémentation.
