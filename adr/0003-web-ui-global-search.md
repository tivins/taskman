# ADR-0003: Global search in Taskman web UI

**Date**: 2026-02-01  
**Status**: Accepted  
**Deciders**: Project team (phase 0 web front)  
**Tags**: front-end, search, API, performance

## Context

The web front audit ([docs/internal/front-web-audit.md](../docs/internal/front-web-audit.md)) and todo list ([docs/internal/front-web-todo.md](../docs/internal/front-web-todo.md)) plan for **global search** (title, description, ID) in the UI. A decision is needed on where search is performed:

- **Client-side**: filter data already loaded in memory (current page tasks or a limited set).
- **Server-side**: new API parameter (e.g. `GET /tasks?search=...`); the backend filters and returns results.

Currently, the tasks API does not expose a `search` parameter; the front loads tasks with filters (phase, milestone, role, status, etc.) and pagination.

## Decision

**Global search will be implemented server-side** via a query parameter on the tasks API (e.g. `GET /tasks?search=...`) when the feature is developed.

- **Search scope**: at least title and ID; ideally description as well (and other text fields if the API exposes them).
- **Behaviour**: the UI sends the search term in requests (e.g. `search=foo`); the server filters tasks and returns paginated results. The front displays these as the current list (with pagination, sort, etc.).
- **During transition**: if global search is shipped before the API supports `search`, a **temporary client-side solution** (filtering on already loaded data, e.g. current page or limited set) is acceptable, provided it is **documented as temporary** and the move to server-side search is planned (ticket or dedicated step).

## Justification

- **Audit**: “Server-side search is preferable once the number of tasks exceeds a few hundred.” Taskman can handle projects with hundreds of tasks; server-side search avoids loading everything into memory and stays performant.
- **Consistency**: other filters (phase, role, status) are already server-side; search fits naturally as an additional filter.
- **Scalability**: full-text search, indexing, scoring, etc. are more realistic on the server.

## Consequences

### Positive

- Performance and scalability independent of task count on the client.
- Alignment with the rest of the filters (API as source of truth).
- Room to enhance later (full-text, additional fields) without front overhaul.

### Negative

- Requires API evolution (new parameter, backend implementation). No front change until the API is ready.
- A temporary client-side version can create behavioural divergence (search limited to loaded data) if it stays in place too long.

### Mitigations

- Specify the parameter (name, encoding, target fields) in the API doc or a backend ADR when implementation is done.
- If a temporary client solution is shipped: document it clearly (comment, todo, ticket) and plan the switch to the API.

## Alternatives considered

### Client-side only search

- **Pros**: no API change, quick to add.
- **Cons**: does not scale with volume (load everything or limit to current page). **Rejected** as target solution.

### Hybrid search (client for small datasets, server beyond)

- **Pros**: optimisation depending on project size.
- **Cons**: complexity and different behaviour by context. **Rejected** to keep a simple rule: search = server.

## References

- Web front audit: [docs/internal/front-web-audit.md](../docs/internal/front-web-audit.md) (sections 2.2, 3.2, 6 — API evolution)
- Todo list: [docs/internal/front-web-todo.md](../docs/internal/front-web-todo.md) (step 0.3)
