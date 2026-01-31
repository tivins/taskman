# Taskman Web Interface

The **Taskman** web interface provides a browser-based UI to view and manage your project tasks, phases, and milestones.

---

## Starting the Web Server

Launch the HTTP server to access the web interface from your browser.

```bash
taskman web # default: 127.0.0.1:8080
taskman web --host 0.0.0.0 --port 3000
taskman web --serve-assets-from embed/web   # dev: serve CSS/JS from disk, edit and refresh without recompile
```

| Option                 | Description                               | Default      |
|------------------------|-------------------------------------------|--------------|
| `--host`               | Listen address                            | `127.0.0.1`  |
| `--port`               | Port (1–65535)                            | `8080`       |
| `--serve-assets-from`  | Serve CSS/JS from directory (dev mode)    | *(embedded)* |

### Development (faster UI iteration)

Use `--serve-assets-from <dir>` so the server reads CSS and JS from the given directory instead of the embedded build. Example from the project root:

```bash
taskman web --serve-assets-from embed/web
```

Edit `embed/web/style.css`, `embed/web/main.js`, etc., then refresh the browser — no recompile needed. The binary still embeds assets; this option only changes how they are served at runtime.

---

## Static Assets

### GET /

Serves the main HTML page with embedded assets. The page includes:
- `style.css` - Stylesheet
- `filters.js` - Filtering module (type=module)
- `pagination.js` - Pagination module (type=module)
- `main.js` - Main application module (type=module)

### GET /style.css

Returns the CSS stylesheet.

### GET /filters.js

Returns the JavaScript filtering module.

### GET /pagination.js

Returns the JavaScript pagination module.

### GET /main.js

Returns the main JavaScript application module.

---

## Task Endpoints

### GET /task/:id

Returns a single task in JSON format. Returns 404 if the task is not found.

**Response:** JSON object with task fields (id, phase_id, milestone_id, title, description, status, sort_order, role).

### GET /task/:id/deps

Returns the dependencies for a specific task.

**Response:** JSON array of objects with `task_id` and `depends_on` fields. Returns 404 if the task is not found.

### GET /task/:id/notes

Returns the notes history for a specific task (notes added by the agent during execution).

**Response:** JSON array of objects with `id`, `task_id`, `content`, `kind`, `role`, `created_at`. Ordered by `created_at`. Returns 404 if the task is not found. The task detail view uses this endpoint to display the "Historique des notes" section.

### GET /notes

Returns notes by a comma-separated list of note IDs (e.g. from `task:get` or `/task/:id` response `note_ids`).

**Query parameters:**

| Parameter | Description                         | Required |
|-----------|-------------------------------------|----------|
| `ids`     | Comma-separated note IDs            | Yes      |

**Response:** JSON array of objects with `id`, `task_id`, `content`, `kind`, `role`, `created_at`. Ordered by `created_at`. Non-existent IDs are skipped; empty `ids` returns an empty array. Returns 400 if `ids` is missing.

### GET /tasks

Returns a paginated list of tasks in JSON format.

**Query parameters:**

| Parameter   | Description                                    | Default | Range      |
|-------------|------------------------------------------------|---------|------------|
| `limit`     | Number of tasks per page                       | `50`    | 1–200      |
| `page`      | Page number (1-based)                          | `1`     | ≥1         |
| `phase`     | Filter by phase ID (optional)                  | —       | —          |
| `milestone` | Filter by milestone ID (optional)              | —       | —          |
| `status`    | Filter by status (optional)                    | —       | See below  |
| `role`      | Filter by role (optional)                      | —       | See below  |

**Valid status values:** `to_do`, `in_progress`, `done`

**Valid role values:** `project-manager`, `project-designer`, `software-architect`, `developer`, `summary-writer`, `documentation-writer`

**Response:** JSON array of task objects, ordered by `phase_id`, `sort_order`, `id`.

### GET /tasks/count

Returns the total count of tasks matching the specified filters.

**Query parameters:** Same as `/tasks` (phase, milestone, status, role).

**Response:** JSON object with `count` field.

### GET /task_deps

Returns a paginated list of task dependencies.

**Query parameters:**

| Parameter | Description                    | Default | Range      |
|-----------|--------------------------------|---------|------------|
| `limit`   | Number of dependencies per page | `100`   | 1–500      |
| `page`    | Page number (1-based)           | `1`     | ≥1         |
| `task_id` | Filter by task ID (optional)    | —       | —          |

**Response:** JSON array of objects with `task_id` and `depends_on` fields, ordered by `task_id`, `depends_on`.

---

## Milestone Endpoints

### GET /milestone/:id

Returns a single milestone in JSON format. Returns 404 if the milestone is not found.

**Response:** JSON object with milestone fields (id, phase_id, name, criterion, reached).

### GET /milestones

Returns a paginated list of milestones in JSON format.

**Query parameters:**

| Parameter | Description                    | Default | Range |
|-----------|--------------------------------|---------|-------|
| `limit`   | Number of milestones per page   | `30`    | 1–100 |
| `page`    | Page number (1-based)           | `1`     | ≥1    |

**Response:** JSON array of milestone objects, ordered by `phase_id`, `id`.

---

## Phase Endpoints

### GET /phase/:id

Returns a single phase in JSON format. Returns 404 if the phase is not found.

**Response:** JSON object with phase fields (id, name, status, sort_order).

### GET /phases

Returns a paginated list of phases in JSON format.

**Query parameters:**

| Parameter | Description                  | Default | Range |
|-----------|------------------------------|---------|-------|
| `limit`   | Number of phases per page    | `30`    | 1–100 |
| `page`    | Page number (1-based)        | `1`     | ≥1    |

**Response:** JSON array of phase objects, ordered by `sort_order`.

---

## Features

The web interface provides:

- **Task listing** with pagination
- **Filtering** by phase, milestone, status, and role
- **Task details** view with dependencies and notes history
- **Notes by IDs** — `GET /notes?ids=...` to fetch multiple notes in one request (e.g. after getting note IDs from a task)
- **Milestone and phase** browsing
- **Responsive design** for desktop and mobile

---

See also: [README](../README.md), [CLI Guide](usage_cli.md), [MCP Server](usage_mcp.md). Users: [User changelog](changelog_user.md). Developers: [build](build.md), [CHANGELOG](../CHANGELOG.md).
