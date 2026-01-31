# Taskman MCP Server

**Taskman** can run as an **MCP (Model Context Protocol) server** for integration with AI assistants like Cursor. The server reads JSON-RPC requests from stdin and writes responses to stdout.

---

## Usage

```bash
taskman mcp
```

The server runs in stdio mode: it reads one JSON-RPC request per line from stdin and writes one JSON-RPC response per line to stdout. The server continues until stdin is closed (EOF).

---

## MCP Protocol

The server implements the MCP specification (version `2025-11-25`):

- **`initialize`**: Handshake with protocol version and server info
- **`notifications/initialized`**: Notification after initialization
- **`tools/list`**: Returns the list of 19 available tools
- **`tools/call`**: Executes a tool with JSON arguments
- **`ping`**: Health check (returns empty result)

---

## Available Tools

All CLI commands (except `web`) are exposed as MCP tools with the `taskman_` prefix:

| CLI Command       | MCP Tool Name               |
|-------------------|-----------------------------|
| `init`            | `taskman_init`              |
| `project:init`    | `taskman_project_init`      |
| `phase:add`       | `taskman_phase_add`         |
| `phase:edit`      | `taskman_phase_edit`       |
| `phase:list`      | `taskman_phase_list`       |
| `milestone:add`   | `taskman_milestone_add`    |
| `milestone:edit`  | `taskman_milestone_edit`   |
| `milestone:list`  | `taskman_milestone_list`   |
| `task:add`        | `taskman_task_add`         |
| `task:get`        | `taskman_task_get`         |
| `task:list`       | `taskman_task_list`        |
| `task:edit`       | `taskman_task_edit`        |
| `task:dep:add`    | `taskman_task_dep_add`     |
| `task:dep:remove` | `taskman_task_dep_remove`  |
| `task:note:add`   | `taskman_task_note_add`    |
| `task:note:list`  | `taskman_task_note_list`   |
| `task:note:list-by-ids` | `taskman_task_note_list_by_ids` |
| `demo:generate`   | `taskman_demo_generate`    |
| `rules:generate`  | `taskman_rules_generate`   |
| `agents:generate` | `taskman_agents_generate`   |

Each tool accepts the same parameters as its CLI counterpart, passed as JSON in the `arguments` object of `tools/call`.

---

## Quick start (new project)

1. **Download taskman** — [Latest release](https://github.com/tivins/taskman/releases).
2. **Bootstrap** — From your project root, run taskman by its full path (not installed on the system nor in PATH):
   ```bash
   /path/to/taskman project:init
   ```
   On Windows: `C:\path\to\taskman.exe project:init`. This runs (in order): `mcp:config`, `init`, `rules:generate`, `agents:generate`.
3. **Reload Cursor** — So the MCP server is loaded.
4. **Use Taskman via the agent** — Phases, milestones, tasks, notes.

---

## Configuration in Cursor

To use taskman as an MCP server in Cursor, you can either:

### Option 1: Automatic configuration (recommended)

Use the `mcp:config` command to automatically generate or update the MCP configuration file:

```bash
taskman mcp:config --executable /path/to/taskman
```

Or on Windows:

```bash
taskman mcp:config --executable C:\path\to\taskman.exe
```

This command will:
- Create or update `.cursor/mcp.json` (or the file specified with `--output`)
- Use the current directory + `project_tasks.db` for `TASKMAN_DB_NAME`
- Set `TASKMAN_JOURNAL_MEMORY=1` automatically
- Merge with existing MCP servers in the file

### Option 2: Manual configuration

Add taskman to your MCP configuration file manually (typically `~/.cursor/mcp.json` or similar):

```json
{
  "mcpServers": {
    "taskman": {
      "command": "taskman",
      "args": ["mcp"],
      "env": {
        "TASKMAN_DB_NAME": "project_tasks.db",
        "TASKMAN_JOURNAL_MEMORY": "1"
      }
    }
  }
}
```

**Environment variables:**

- `TASKMAN_DB_NAME`: Path to the SQLite database file (default: `project_tasks.db`)
- `TASKMAN_JOURNAL_MEMORY`: Set to `1` to use an in-memory journal (recommended when running from Cursor agent to avoid disk I/O errors)
- `CURSOR_AGENT`: When set by Cursor, taskman automatically uses an in-memory journal

**Note:** The `command` path should be either:
- An absolute path to the `taskman` executable
- A command available in your system PATH

---

## Example: Manual Testing

You can test the MCP server manually:

```bash
# Initialize request
echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"test","version":"1.0"}}}' | taskman mcp

# List tools
echo '{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}' | taskman mcp

# Call a tool (initialize database)
echo '{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"taskman_init","arguments":{}}}' | taskman mcp
```

---

## Error Handling

- **Protocol errors** (invalid JSON, unknown method, unsupported protocol version): Returned as JSON-RPC errors with standard error codes (`-32700` Parse error, `-32600` Invalid Request, `-32601` Method not found, `-32602` Invalid params).
- **Business logic errors** (task not found, invalid phase, etc.): Returned in the `result` with `isError: true` and the error message in `content[0].text`.

---

See also: [README](../README.md), [CLI Guide](usage_cli.md), [Web Interface](usage_web.md). Users: [User changelog](changelog_user.md). Developers: [build](build.md), [CHANGELOG](../CHANGELOG.md).
