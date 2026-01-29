# Report: Human Users of Taskman via Agents

**Context**: Taskman is not intended to be used directly by humans, but by AI agents. This document summarises a discussion on the types of human users who use Taskman *via* these agents.

---

## 1. Important Clarification

Taskman is designed to be **driven by AI agents** (MCP in Cursor, or any MCP client). Humans do not use Taskman by hand; they go through the agent. The “roles” (developer, project-manager, product-owner, etc.) are **agent personas**: the agent filters and creates tasks with a given role. **Human users** are the people who address the AI assistant and ask it to act on Taskman.

---

## 2. Identified Human User Types

### 2.1 Developers (Cursor first)

- **Profile**: Developer coding in Cursor, with Taskman MCP configured.
- **Usage**: They ask the agent (in “developer” mode) to list their tasks, mark them in progress/done, create dev or unassigned tasks.
- **Why**: No need to leave the IDE or type CLI commands; tracking stays in the same tool as the code.
- **Note**: This is the most direct use case given the current documentation (Cursor MCP).

### 2.2 Project Managers

- **Profile**: PM using an AI assistant (Cursor or another MCP client).
- **Usage**: They ask the “project-manager” agent to create phases and milestones, assign tasks to roles, validate unassigned tasks, track progress.
- **Why**: Planning and coordination delegated to the agent; the human provides intent, the agent executes in Taskman.
- **Note**: Especially relevant if the PM is comfortable with Cursor or another MCP client.

### 2.3 Product Owners

- **Profile**: PO working with an AI assistant.
- **Usage**: “product-owner” agent to view and update their tasks (backlog, acceptance criteria, prioritisation), create PO or unassigned tasks.
- **Why**: Backlog/task alignment without touching the CLI or another UI.

### 2.4 Other Business or Technical Roles

The roles in `embed/roles_agents/` (architect, QA, UX/UI, doc, etc.) correspond to **agent personas**. Humans using them would be, for example:

- **Tech leads / architects**: use the “software-architect” agent for architecture tasks and dependencies.
- **QA**: use the “qa-engineer” agent for test and validation tasks.
- **Designers (UI/UX, art direction)**: use the ui-designer, ux-designer, art-director agents for their tasks.
- **Doc writers / community**: use “documentation-writer” or “community-manager”.

All rely on an **AI assistant** (often in Cursor) and a **Taskman role agent** so they do not use Taskman directly.

### 2.5 One Person, Multiple Roles

The same human (e.g. a developer who also does PM on a small project) can ask the agent to act sometimes as “developer”, sometimes as “project-manager”. “User types” are therefore more like **need types** (dev, PM, PO, QA, etc.) than strict user accounts.

---

## 3. Summary

| Human user type       | Typical context                    | Most used Taskman agent              |
|-----------------------|------------------------------------|--------------------------------------|
| **Developer**         | Cursor, day-to-day work            | developer                            |
| **Project manager**   | Cursor or other MCP client         | project-manager                      |
| **Product owner**     | Same                               | product-owner                        |
| **Tech lead / Architect** | Cursor, technical design       | software-architect                   |
| **QA**                | Cursor or other                    | qa-engineer                          |
| **Design (UI/UX/Art)**| Same                               | ui-designer, ux-designer, art-director |
| **Doc / Community**   | Same                               | documentation-writer, community-manager |

In practice, **Cursor developers** are the most natural human users today (Cursor MCP documented, code + task workflow in the same tool). **Project managers** and **product owners** become “users” as soon as they use an MCP client with the corresponding Taskman agents. The other roles extend the same model to other profiles (architect, QA, design, doc) depending on how mature AI assistant usage is in the team.

---

See also: [usage.md](usage.md) (detailed overview of Taskman usage).
