# Taskman Usage — Detailed Overview

This document describes in detail how Taskman is used: data model, interfaces (CLI, MCP, Web), workflows, and task assignment rules. For per-interface user guides, see [USAGE_CLI.md](USAGE_CLI.md), [USAGE_MCP.md](USAGE_MCP.md), and [USAGE_WEB.md](USAGE_WEB.md).

---

## 1. Overview

Taskman is a project management tool (phases, milestones, tasks with dependencies) stored in an SQLite database. It is designed to be **used by AI agents** (via MCP) rather than directly by humans; humans interact with Taskman through the assistant (e.g. Cursor).

| Interface | Primary use | Audience |
|-----------|------------------|--------|
| **CLI** | Scripts, pipelines, manual testing | Developers, automation |
| **MCP** | Integration with Cursor and other MCP clients | AI agents on behalf of humans (dev, PM, PO, etc.) |
| **Web** | Browsing and filtering (read-only) | Humans for overview; no documented editing |

---

## 2. Data Model

### 2.1 Phases

- **Role**: Structure the project (e.g. Design, Development, Acceptance, Delivery).
- **Fields**: `id` (e.g. P1, P2), `name`, `status` (`to_do` | `in_progress` | `done`), `sort_order`.
- **Usage**: Create and update via CLI or MCP; browse via CLI, MCP, or Web.

### 2.2 Milestones

- **Role**: Goals tied to a phase (e.g. “Specifications approved”, “MVP delivered”).
- **Fields**: `id`, `phase_id`, `name`, `criterion`, `reached` (0/1).
- **Usage**: Same — CLI/MCP for editing, all interfaces for reading.

### 2.3 Tasks

- **Role**: Unit of work, linked to a phase and optionally to a milestone.
- **Fields**: `id` (UUID v4), `phase_id`, `milestone_id` (optional), `title`, `description`, `status`, `sort_order`, `role`.
- **Statuses**: `to_do`, `in_progress`, `done`.
- **Roles** (list defined in code, `src/util/roles.cpp`):  
  `project-manager`, `project-designer`, `software-architect`, `developer`, `summary-writer`, `documentation-writer`, `art-director`, `ui-designer`, `community-manager`, `ux-designer`, `qa-engineer`, `devops-engineer`, `product-owner`, `security-engineer`.
- **Dependencies**: A task can depend on other tasks (`task_deps`: `task_id` → `depends_on`). Both must exist; a task cannot depend on itself.
- **Notes**: Tasks can have notes (`task_notes`: content, `kind` completion/progress/issue, `role`, `created_at`) to record agent work history.

### 2.4 Database

- **File**: By default `project_tasks.db` in the current directory; configurable via `TASKMAN_DB_NAME`.
- **Initialization**: `taskman init` (or MCP tool `taskman_init`) creates the tables if needed. For a full bootstrap (MCP config, DB, rules, agents), use `taskman project:init --executable <path>` (or `taskman_project_init` via MCP).
- **Demo**: `taskman demo:generate` (or `taskman_demo_generate`) recreates a sample database (phases, milestones, tasks, dependencies, notes).

---

## 3. Usage by Interface

### 3.1 CLI

- **When**: Scripts, CI, tests, one-off terminal operations.
- **Commands**: See [USAGE_CLI.md](USAGE_CLI.md) (summary at end of section 7). All operations (init, phases, milestones, tasks, deps, notes, demo) are available.
- **Formats**: JSON (default for lists/details) or `text` for tasks (human-readable).
- **Environment variables**: `TASKMAN_DB_NAME`, `TASKMAN_JOURNAL_MEMORY` (recommended `1` in agent/sandbox context), `CURSOR_AGENT` (Cursor sets journal in memory automatically).

### 3.2 MCP (AI agents)

- **When**: Normal use by an AI assistant (e.g. Cursor) on behalf of a human.
- **Protocol**: MCP in stdio mode; the server reads JSON-RPC from stdin and writes responses to stdout.
- **Tools**: Each CLI command (except `web`) is exposed as a tool with the `taskman_` prefix (e.g. `taskman_task_list`, `taskman_phase_add`, `taskman_rules_generate`, `taskman_agents_generate`, `taskman_project_init`). Arguments match the CLI and are passed as JSON in `arguments` of `tools/call`.
- **Cursor configuration**: `taskman mcp:config --executable <path>` or manual configuration in the MCP file (e.g. `~/.cursor/mcp.json`). Details in [USAGE_MCP.md](USAGE_MCP.md).
- **Role agents**: Files in `embed/roles_agents/` describe how each role uses MCP (role filters, common actions, creating assigned or unassigned tasks). See [Agents README](../embed/roles_agents/README.md).

### 3.3 Web

- **When**: Human browsing (task list, filters, task detail, dependencies, notes).
- **Starting**: `taskman web` (optionally `--host`, `--port`, `--serve-assets-from` for prod/dev).
- **Features**: Pagination, filters (phase, milestone, status, role), task detail with deps and notes. No documented editing in USAGE_WEB; editing remains via CLI/MCP.
- **Endpoint details**: [USAGE_WEB.md](USAGE_WEB.md).

---

## 4. Typical Workflows

### 4.1 First use (bootstrap a new project)

1. Download taskman from the [latest release](https://github.com/tivins/taskman/releases).
2. From the project root, run: `taskman project:init --executable <path-to-taskman>`. This writes `.cursor/mcp.json`, creates the database, and generates `.cursor/rules/` and `.cursor/agents/`.
3. Reload Cursor so the MCP server is loaded.
4. Use Taskman via the agent (phases, milestones, tasks).

### 4.2 First use (demo)

1. Generate the demo database: `taskman demo:generate` (or `taskman_demo_generate` via MCP).
2. Browse phases / milestones / tasks: CLI `phase:list`, `milestone:list`, `task:list` or equivalent MCP tools, or Web interface.

### 4.3 New project (empty database, manual setup)

1. `taskman init`.
2. Create phases (`phase:add` / `taskman_phase_add`).
3. Create milestones (`milestone:add` / `taskman_milestone_add`).
4. Create tasks (`task:add` / `taskman_task_add`), optionally add dependencies (`task:dep:add` / `taskman_task_dep_add`).

### 4.4 Use by an agent on behalf of a role

1. The human configures the Taskman MCP and enables the role agent (e.g. developer, project-manager) in Cursor.
2. The agent lists tasks for that role: `taskman_task_list` with `role: "<role>"` (and optionally `phase`, `status`).
3. For a given task: `taskman_task_get`, then update status or description via `taskman_task_edit`, add notes via `taskman_task_note_add`.
4. Creating tasks: either with `role: "<role>"` (assigned to the role) or without `role` (unassigned task, to be validated by the project manager).

### 4.5 Project manager role

- Create phases and milestones.
- Create tasks assigned to **any role** (including themselves).
- List tasks without a `role` filter to find **unassigned** tasks, then validate and assign via `taskman_task_edit` with the appropriate `role`.

---

## 5. Task Assignment Rules

- **Project manager**: May assign tasks to **all roles** (including themselves).
- **Other roles**: May create tasks:
  - assigned to their own role (`role: "<role>"`), or
  - unassigned (no `role` parameter) for validation and assignment by the project manager.
- **Validation**: The project manager should regularly list tasks without a role, validate them, edit if needed, then assign them to the appropriate role.

These rules are carried by the agents in `embed/roles_agents/` (expected behaviour and example MCP calls).

---

## 6. Environment Variables (Summary)

| Variable | Description | Default |
|----------|-------------|--------|
| `TASKMAN_DB_NAME` | Path to the SQLite file | `project_tasks.db` |
| `TASKMAN_JOURNAL_MEMORY` | `1` = in-memory journal (avoids I/O errors in sandbox/agent) | not set |
| `CURSOR_AGENT` | Set by Cursor; Taskman then uses in-memory journal | — |

---

## 7. References

- **User guides**: [USAGE_CLI.md](USAGE_CLI.md), [USAGE_MCP.md](USAGE_MCP.md), [USAGE_WEB.md](USAGE_WEB.md).
- **Build**: [BUILD.md](BUILD.md).
- **Human users via agents**: [users.md](users.md).
