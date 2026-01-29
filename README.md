# Taskman

Project management for AI-assisted workflows: phases, milestones, and role-assigned tasks with dependencies, stored in SQLite. Agents (via MCP) create and update work; humans interact through Cursor or other MCP clients. The CLI is for scripts and automation, the web UI for read-only browsing; output is JSON or plain text. It runs as a standalone C++ binary on Windows, Linux, and macOS.

## Documentation

* [Human users via agents](docs/users.md): who uses Taskman and which agent roles
* [Usage overview](docs/usage.md): data model, interfaces, workflows, task assignment
* [MCP Server](docs/USAGE_MCP.md): integration with AI assistants
* [CLI Usage](docs/USAGE_CLI.md): command-line interface, commands, options, formats
* [Web Interface](docs/USAGE_WEB.md): browser-based UI (read-only - edit planned)
* [Build](docs/BUILD.md): download, build and run instructions

## Changes

* [User changelog](docs/CHANGELOG_USER.md) (non-developer changes)
* [Global Changelog](CHANGELOG.md) (all changes)
