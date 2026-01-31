# ADR-0002: Navigation and state in Taskman web UI

**Date**: 2026-02-01  
**Status**: Accepted  
**Deciders**: Project team (phase 0 web front)  
**Tags**: front-end, navigation, URL, state, SPA, deep links

## Context

The web front audit ([docs/internal/front-web-audit.md](../docs/internal/front-web-audit.md)) and todo list ([docs/internal/front-web-todo.md](../docs/internal/front-web-todo.md)) identify the need for:

1. **URL-persisted state**: current view, filters (phase, milestone, role, status, blocked), pagination page, sort — to allow link sharing and returning to a specific context (refresh, closed then reopened tab).
2. **Deep links to a task**: share a direct link to a task (e.g. `#/task/abc-123`).
3. **Task detail behaviour**: when clicking a task in the list, should the URL change (dedicated route) or open a side panel (peek) without changing the URL? Both approaches exist (Linear/Plane use route + peek); for Taskman, a clear decision simplifies implementation.

Currently, the front does not use browser history (`history.pushState` / hash), there are no deep links, and no persistence of filters/page in the URL.

## Decision

### 1. URL as source of truth for view and list state

**The URL (hash + query) is the source of truth** for:

- **Current view**: list, board, dashboard, etc. (e.g. `#/list`, `#/board`, `#/` for dashboard).
- **List state** (when list view is active): `page`, `phase`, `milestone`, `role`, `status`, `blocked_filter`, `sort`, `group_by`, and any filter or sort parameter exposed in the UI.

**Implementation**:

- Use the **hash** for routes (e.g. `#/list`, `#/task/:id`) so as not to depend on server configuration (the front is served statically).
- Use the document **query string** (or within the hash if everything is in the hash, e.g. `#/list?page=2&phase=P1`) for list parameters. Chosen format: **`#/list?page=2&phase=P1&status=in_progress`** (parameters in the query, route in the hash path).
- On **load**: parse `location.hash` and `location.search` (or the query within the hash), restore view and state (filters, page, sort).
- On each **view, filter, page or sort change**: update the URL via `history.pushState` (user navigation) or `replaceState` (silent correction), and listen to `popstate` to react to the browser back/forward buttons.

This enables shareable links, bookmarks, and preserving context after refresh.

### 2. Task detail: peek by default, route for sharing

**Chosen behaviour**:

- **Click on a task in the list**: open the detail in a **right-side panel (peek)** without changing the URL. The list, filters and page remain visible; the user can close the panel and get back to the same state.
- **Sharing and deep link**: a **dedicated route** `#/task/:id` exists. It is used:
  - when the user chooses “Open in URL” (or “Copy link”) from the detail panel;
  - when the user lands on the app with a URL like `...#/task/:id` (received link, bookmark).
- **When the URL is `#/task/:id`**: show the task either in the same peek panel (with the list in the background if on list view), or in the main area if readability of the shared link is preferred. Implementation decision: **show the task in the peek panel** with the list behind; the list can be loaded with default filters or no particular state, unless “last visited list” is also persisted (out of scope for this ADR).

In short: **default interaction = peek without URL change; sharing and direct entry = route `#/task/:id`**.

## Justification

- **URL as source of truth**: alignment with common SPA practices, link sharing and context restoration without server-side state.
- **Hash for routes**: simple deployment (static files, no server rewrite).
- **Peek by default**: fewer “back” actions, context preserved (page, filters), smooth experience for list navigation.
- **Route `#/task/:id`**: required for sharing and deep links; a single dedicated task route is enough, without duplicating logic between “full-screen view” and “peek”.

## Consequences

### Positive

- Shareable links for the list (filters, page) and for a task.
- Predictable behaviour (browser back button consistent with URL).
- Peek avoids losing list context.

### Negative

- Initial implementation: handling `popstate`, bidirectional state ↔ URL sync, parsing on load.
- Two ways to show a task (peek from list, or URL `#/task/:id`) to keep consistent.

### Mitigations

- Centralise “read / write URL” logic in a small module (router/state-from-url) to avoid duplication.
- Test explicitly: task click → peek; “Open in URL” → URL updated; load with `#/task/:id` → task displayed.

## Alternatives considered

### URL: everything in hash vs path + query

- **Everything in hash** (e.g. `#/list?page=2`): no server request, simple for a static site. **Chosen.**
- **HTML5 path** (e.g. `/list`, `/task/123`): “cleaner” URLs but requires server configuration (fallback to index.html). **Rejected** for now to keep static deployment simple.

### Task detail: peek only vs route only vs both

- **Peek only**: no deep link possible. **Rejected.**
- **Route only**: every click changes the URL and loses “position” in the list (unless everything is persisted in the URL). **Rejected** to favour smoothness.
- **Peek by default + route for sharing**: chosen compromise, aligned with the audit (section 6) and the todo (step 0.2).

## References

- Web front audit: [docs/internal/front-web-audit.md](../docs/internal/front-web-audit.md) (sections 2.2, 3.9, 4, 6)
- Todo list: [docs/internal/front-web-todo.md](../docs/internal/front-web-todo.md) (step 0)
