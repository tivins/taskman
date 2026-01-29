# Taskman CLI User Guide

**Taskman** is a command-line tool to manage project phases, milestones, and tasks in a SQLite database. It outputs JSON or plain text, suitable for integration with agents (project-manager, developers, writers, etc.).

For build and run instructions, see [BUILD.md](BUILD.md).

---

## 1. Database and initialization

### Environment variable

| Variable                | Description                                                                 | Default            |
|-------------------------|-----------------------------------------------------------------------------|--------------------|
| `TASKMAN_DB_NAME`       | Path to the SQLite database file                                            | `project_tasks.db` |
| `TASKMAN_JOURNAL_MEMORY`| Set to `1` to use an in-memory journal (avoids "disk I/O error" in sandboxes, e.g. Cursor agent) | not set            |
| `CURSOR_AGENT`          | When set by Cursor, taskman uses an in-memory journal automatically         | —                  |

Examples (bash):

```bash
export TASKMAN_DB_NAME=./my_project.db
# or inline:
TASKMAN_DB_NAME=./other.db taskman phase:list
```

On Windows (PowerShell):

```powershell
$env:TASKMAN_DB_NAME = ".\my_project.db"
```

### Initialize the database

Creates the `phases`, `milestones`, `tasks`, `task_deps`, and `task_notes` tables (if they do not exist). Run once when starting a new project.

```bash
taskman init
```

If errors occur (e.g. corrupted database or orphan `.db-journal`), see [BUILD.md](BUILD.md). When using taskman from **Cursor's agent**, `TASKMAN_JOURNAL_MEMORY=1` often fixes "disk I/O error" (see BUILD.md).

### Bootstrap a new project (`project:init`)

Runs in order: `mcp:config` (if `--executable` is given), `init`, `rules:generate`, `agents:generate`. Use this to set up a new project for use with Cursor and the agent. Then reload Cursor so the MCP server is loaded.

```bash
taskman project:init --executable /path/to/taskman
```

Without `--executable`, MCP config is skipped; run `taskman mcp:config --executable <path>` separately to configure Cursor.

### Generate a demo database

Creates a database filled with a realistic example (e-commerce site MVP project) for demonstration or testing purposes. The command automatically removes any existing database file (and related journal files) before creating the new one.

```bash
taskman demo:generate
```

The demo database includes:
- **4 phases**: Design (in progress), Development, Acceptance, Delivery
- **4 milestones**: Specs approved (reached), MVP delivered, Acceptance OK, Production deployment
- **31 tasks** with priorities (sort_order), roles, and realistic statuses (some done, some in progress, some to do)
- **Dependencies** between tasks
- **Notes** on several tasks (completion and progress kinds)

You can specify a custom database path using the `TASKMAN_DB_NAME` environment variable:

```bash
TASKMAN_DB_NAME=./demo.db taskman demo:generate
```

---

## 2. Phases

**Phases** structure the project (e.g. design, development, acceptance). Each has an id, name, status, and display order.

### `phase:add` — Create a phase

```bash
taskman phase:add --id <id> --name <name> [--status to_do|in_progress|done] [--sort-order <n>]
```

| Option        | Required | Description                                    | Default  |
|---------------|----------|------------------------------------------------|----------|
| `--id`        | yes      | Unique phase identifier                        | —        |
| `--name`      | yes      | Phase name                                     | —        |
| `--status`    | no       | `to_do`, `in_progress`, or `done`               | `to_do`  |
| `--sort-order`| no       | Integer for display order (`phase:list`)        | —        |

Examples:

```bash
taskman phase:add --id P1 --name "Design"
taskman phase:add --id P2 --name "Development" --status in_progress --sort-order 2
```

### `phase:edit` — Edit a phase

```bash
taskman phase:edit <id> [--name <name>] [--status to_do|in_progress|done] [--sort-order <n>]
```

Only the fields passed as options are updated. At least one option is required for a change to take effect.

Example:

```bash
taskman phase:edit P1 --status in_progress --sort-order 1
```

### `phase:list` — List phases

```bash
taskman phase:list
```

Output: JSON array of phases, sorted by `sort_order`. Each object has: `id`, `name`, `status`, `sort_order` (integer or `null`).

---

## 3. Milestones

**Milestones** are attached to a phase. They describe a goal (name, criterion) and whether it is reached (`reached`).

### `milestone:add` — Create a milestone

```bash
taskman milestone:add --id <id> --phase <phase_id> [--name <name>] [--criterion <text>] [--reached 0|1]
```

| Option       | Required | Description                          | Default |
|--------------|----------|--------------------------------------|---------|
| `--id`       | yes      | Unique milestone identifier          | —       |
| `--phase`    | yes      | Parent phase ID                      | —       |
| `--name`     | no       | Milestone name                       | —       |
| `--criterion`| no       | Success criterion                    | —       |
| `--reached`  | no       | `0` (not reached) or `1` (reached)   | `0`     |

Examples:

```bash
taskman milestone:add --id M1 --phase P1 --name "Specs approved" --criterion "Document signed off"
taskman milestone:add --id M2 --phase P2 --name "MVP delivered" --reached 1
```

### `milestone:edit` — Edit a milestone

```bash
taskman milestone:edit <id> [--name <name>] [--criterion <text>] [--reached 0|1] [--phase <phase_id>]
```

Partial update: only the provided options are changed.

Example:

```bash
taskman milestone:edit M1 --reached 1
```

### `milestone:list` — List milestones

```bash
taskman milestone:list [--phase <phase_id>]
```

| Option   | Description             |
|----------|-------------------------|
| `--phase`| Filter by phase ID      |

Output: JSON array. Fields: `id`, `phase_id`, `name`, `criterion`, `reached` (integer).

---

## 4. Tasks

**Tasks** are linked to a phase and optionally to a milestone. They have a title, description, status, assigned role, and can have dependencies. The ID is an auto-generated UUID v4 on creation.

### Allowed roles

`project-manager`, `project-designer`, `software-architect`, `developer`, `summary-writer`, `documentation-writer`.

### Statuses

`to_do`, `in_progress`, `done`.

### `task:add` — Create a task

```bash
taskman task:add --title <title> --phase <phase_id> [--description <text>] [--role <role>] [--milestone <milestone_id>] [--format json|text]
```

| Option         | Required | Description                              | Default |
|----------------|----------|------------------------------------------|---------|
| `--title`      | yes      | Task title                               | —       |
| `--phase`      | yes      | Phase ID                                 | —       |
| `--description`| no       | Description                              | —       |
| `--role`       | no       | One of the roles listed above            | —       |
| `--milestone`  | no       | Associated milestone ID                  | —       |
| `--format`     | no       | `json` or `text` for the created task   | `json`  |

The task is inserted with status `to_do`. Output shows the created task (same as `task:get`).

Examples:

```bash
taskman task:add --title "Write USAGE.md" --phase P1 --role documentation-writer --format text
taskman task:add --title "Implement login" --phase P2 --milestone M2 --description "API + UI"
```

### `task:get` — Show a task

```bash
taskman task:get <id> [--format json|text]
```

| Option   | Required | Description          | Default |
|----------|----------|----------------------|---------|
| `<id>`   | yes      | Task UUID            | —       |
| `--format`| no      | `json` or `text`     | `json`  |

With `text`, the order is: `title`, `description`, `status`, `role`, then `id`, `phase_id`, `milestone_id`, `sort_order`.

### `task:list` — List tasks

```bash
taskman task:list [--phase <id>] [--status <s>] [--role <r>] [--format json|text]
```

| Option     | Description                                          | Default |
|------------|------------------------------------------------------|---------|
| `--phase`  | Filter by phase ID                                  | —       |
| `--status` | Filter by status (`to_do`, `in_progress`, `done`)   | —       |
| `--role`   | Filter by role                                      | —       |
| `--format` | `json` (array) or `text` (blocks separated by `---`)| `json`  |

Sort order: `phase_id`, `sort_order`, `id`.

Examples:

```bash
taskman task:list --phase P2 --status in_progress
taskman task:list --role documentation-writer --format text
```

### `task:edit` — Edit a task

```bash
taskman task:edit <id> [--title <title>] [--description <text>] [--status to_do|in_progress|done] [--role <role>] [--milestone <milestone_id>]
```

Partial update: only the provided options are changed. At least one option is required for a change to take effect.

Example:

```bash
taskman task:edit <uuid> --status done
```

### `task:dep:add` — Add a dependency

Marks task `<task-id>` as depending on `<dep-id>` (task `dep-id` must be completed first).

```bash
taskman task:dep:add <task-id> <dep-id>
```

- Both tasks must exist.
- A task cannot depend on itself.
- If the dependency already exists, an error is returned.

Example:

```bash
taskman task:dep:add abc-123-def xyz-456-uvw
```

### `task:dep:remove` — Remove a dependency

```bash
taskman task:dep:remove <task-id> <dep-id>
```

### `task:note:add` — Add a note to a task

Adds a note to a task (e.g. completion summary, progress, or issue). The note ID is an auto-generated UUID v4.

```bash
taskman task:note:add <task-id> --content "..." [--kind completion|progress|issue] [--role <role>] [--format json|text]
```

| Option     | Required | Description                                      | Default |
|------------|----------|--------------------------------------------------|---------|
| `<task-id>`| yes      | Task UUID to attach the note to                 | —       |
| `--content`| yes      | Note content                                    | —       |
| `--kind`   | no       | `completion`, `progress`, or `issue`             | —       |
| `--role`   | no       | Role of the agent who added the note (see roles) | —       |
| `--format` | no       | `json` or `text` for the created note           | `json`  |

The task must exist. Output shows the created note (id, task_id, content, kind, role, created_at).

Example:

```bash
taskman task:note:add abc-123-def --content "Done. Fixed lint." --kind completion --role developer
```

### `task:note:list` — List notes for a task

```bash
taskman task:note:list <task-id> [--format json|text]
```

| Option     | Required | Description                         | Default |
|------------|----------|-------------------------------------|---------|
| `<task-id>`| yes      | Task UUID                           | —       |
| `--format`| no       | `json` (array) or `text`            | `json`  |

Notes are ordered by `created_at`. For a non-existent task, returns an empty list.

---

## 5. Output formats

### JSON

- **phases**: `id`, `name`, `status`, `sort_order` (integer or `null`).
- **milestones**: `id`, `phase_id`, `name`, `criterion`, `reached` (integer).
- **tasks**: `id`, `phase_id`, `milestone_id`, `title`, `description`, `status`, `sort_order`, `role`. Empty optional fields are `null`.

Suitable for scripts, pipelines, and integration with other tools.

### Text (tasks only)

Human-readable format: `title`, `description`, `status`, `role`, then `id`, `phase_id`, `milestone_id`, `sort_order`. For `task:list --format text`, tasks are separated by `---`.

---

## 6. Exit codes

| Code | Meaning                                               |
|------|--------------------------------------------------------|
| `0`  | Success                                                |
| `1`  | Error (parsing, database, validation, or arguments)   |

Error messages are written to standard error (`stderr`).

---

## 7. Command summary

| Command           | Description                                  |
|-------------------|----------------------------------------------|
| `init`            | Create / initialize tables                   |
| `project:init`    | Bootstrap: mcp:config, init, rules:generate, agents:generate |
| `phase:add`       | Add a phase                                  |
| `phase:edit`      | Edit a phase                                 |
| `phase:list`      | List phases                                  |
| `milestone:add`   | Add a milestone                              |
| `milestone:edit`  | Edit a milestone                             |
| `milestone:list`  | List milestones (option `--phase`)          |
| `task:add`        | Add a task (auto UUID)                       |
| `task:get`        | Show a task                                  |
| `task:list`       | List tasks (filters, `--format`)             |
| `task:edit`       | Edit a task                                  |
| `task:dep:add`    | Add a task dependency                        |
| `task:dep:remove` | Remove a dependency                          |
| `task:note:add`   | Add a note to a task                         |
| `task:note:list`  | List notes for a task                        |
| `demo:generate`   | Generate a demo database                     |
| `agents:generate`| Generate .cursor/agents/ files (from embedded agents) |
| `rules:generate` | Generate .cursor/rules/ files (from embedded rules)    |

---

## 8. Example workflow

```bash
# Option A: Start with a demo database (recommended for first-time users)
taskman demo:generate

# Option B: Initialize an empty database
# 1. Initialize the database
taskman init

# 2. Create phases
taskman phase:add --id P1 --name "Design" --sort-order 1
taskman phase:add --id P2 --name "Development" --sort-order 2

# 3. Create a milestone
taskman milestone:add --id M1 --phase P1 --name "Specs approved"

# 4. Create tasks
taskman task:add --title "Write the specs" --phase P1 --milestone M1 --role project-designer
taskman task:add --title "Document the API" --phase P2 --role documentation-writer

# 5. List and filter
taskman phase:list
taskman task:list --phase P1 --format text

# 6. Update
taskman task:edit <task-uuid> --status done
taskman milestone:edit M1 --reached 1
```

---

See also: [README](../README.md), [Web Interface](USAGE_WEB.md), [MCP Server](USAGE_MCP.md). Users: [User changelog](CHANGELOG_USER.md). Developers: [BUILD](BUILD.md), [CHANGELOG](../CHANGELOG.md).
