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
- **`tools/list`**: Returns the list of 13 available tools
- **`tools/call`**: Executes a tool with JSON arguments
- **`ping`**: Health check (returns empty result)

---

## Available Tools

All CLI commands (except `web`) are exposed as MCP tools with the `taskman_` prefix:

| CLI Command      | MCP Tool Name              |
|------------------|---------------------------|
| `init`           | `taskman_init`            |
| `phase:add`      | `taskman_phase_add`       |
| `phase:edit`     | `taskman_phase_edit`      |
| `phase:list`     | `taskman_phase_list`      |
| `milestone:add`  | `taskman_milestone_add`   |
| `milestone:edit` | `taskman_milestone_edit`  |
| `milestone:list` | `taskman_milestone_list`  |
| `task:add`       | `taskman_task_add`        |
| `task:get`       | `taskman_task_get`        |
| `task:list`      | `taskman_task_list`       |
| `task:edit`      | `taskman_task_edit`       |
| `task:dep:add`   | `taskman_task_dep_add`    |
| `task:dep:remove`| `taskman_task_dep_remove` |

Each tool accepts the same parameters as its CLI counterpart, passed as JSON in the `arguments` object of `tools/call`.

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

See also: [README](README.md), [CLI Guide](USAGE_CLI.md), [Web Interface](USAGE_WEB.md). Users: [User changelog](CHANGELOG_USER.md). Developers: [BUILD](BUILD.md), [CHANGELOG](CHANGELOG.md).
