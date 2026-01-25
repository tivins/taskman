# Project manager | tasks manager

## But

`taskman` est un exécutable (fichier unique) utilisable uniquement en [CLI](#cli).
Ce programme doit pouvoir être utilisé directement par des agents pour récupérer une tâche et la modifier.

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

## Roles

* project-manager
* project-designer
* software-architect
* developer
* summary-writer
* documentation-writer

## CLI

```shell
taskman init # Initialise la base de SQLite (par défaut 'project_tasks.db', mais surchargeable avec la variable ENV 'TASKMAN_DB_NAME').
taskman task:add --title "" --description "" --role "" --milestone "" --phase ""
taskman task:edit <task-id> --title "modified title" 
taskman task:get <task-id> # output JSON
taskman task:get <task-id> --format "text"
```