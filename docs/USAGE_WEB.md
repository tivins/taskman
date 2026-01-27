# Taskman Web Interface

The **Taskman** web interface provides a browser-based UI to view and manage your project tasks, phases, and milestones.

---

## Starting the Web Server

Launch the HTTP server to access the web interface from your browser.

```bash
taskman web # default: 127.0.0.1:8080
taskman web --host 0.0.0.0 --port 3000
```

| Option    | Description           | Default      |
|-----------|-----------------------|--------------|
| `--host`  | Listen address        | `127.0.0.1`  |
| `--port`  | Port (1–65535)        | `8080`       |

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
- **Task details** view with dependencies
- **Milestone and phase** browsing
- **Responsive design** for desktop and mobile

---

See also: [README](../README.md), [CLI Guide](USAGE_CLI.md), [MCP Server](USAGE_MCP.md). Users: [User changelog](CHANGELOG_USER.md). Developers: [BUILD](BUILD.md), [CHANGELOG](../CHANGELOG.md).
