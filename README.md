# Taskman

Project management for AI-assisted workflows: phases, milestones, and role-assigned tasks with dependencies, stored in SQLite. Agents (via MCP) create and update work; humans interact through Cursor or other MCP clients. The CLI is for scripts and automation, the web UI for read-only browsing; output is JSON or plain text. It runs as a standalone C++ binary on Windows, Linux, and macOS.

## Quick start

1. **Download taskman** — [Latest release](https://github.com/tivins/taskman/releases) (Windows, Linux, macOS).
2. **Bootstrap the project** — From your project root, run taskman by its full path:
   ```bash
   /path/to/taskman project:init
   ```
   This writes `.cursor/mcp.json`, creates the database, and generates `.cursor/rules/` and `.cursor/agents/`.
3. **Reload Cursor** — So the MCP server is loaded.
4. **Use Taskman via the agent** — Ask the agent to manage phases, milestones, and tasks.

## Documentation

* [Human users via agents](docs/users.md): who uses Taskman and which agent roles
* [Usage overview](docs/usage.md): data model, interfaces, workflows, task assignment
* [MCP Server](docs/usage_mcp.md): integration with AI assistants
* [CLI Usage](docs/usage_cli.md): command-line interface, commands, options, formats
* [Web Interface](docs/usage_web.md): browser-based UI (read-only - edit planned)
* [Build](docs/build.md): download, build and run instructions

## Changes

* [User changelog](docs/changelog_user.md) (non-developer changes)
* [Global Changelog](CHANGELOG.md) (all changes)
