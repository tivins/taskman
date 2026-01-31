# ADR-0001: Defer implementation of abstract interfaces and dependency injection

**Date**: 2026-01-28  
**Status**: Accepted  
**Deciders**: [@Tivins](https://github.com/tivins), Development team  
**Tags**: architecture, SOLID, YAGNI, dependencies

## Context

During SOLID analysis of the code (see `docs/internal/analysis-2026-01-28-solid.md`), several violations of the Dependency Inversion Principle (DIP) were identified:

1. **Direct dependencies on concrete implementations**:
   - `Database` is directly coupled to SQLite
   - Repositories (`TaskRepository`, `PhaseRepository`, etc.) are concrete classes without abstract interfaces
   - `WebServer` depends directly on `httplib::Server`
   - Command parsers depend directly on `cxxopts::Options`

2. **Direct instantiation in code**:
   - `main.cpp` directly instantiates `Database`
   - `web.cpp` directly instantiates `WebServer` and controllers
   - `mcp.cpp` directly instantiates handlers

3. **Impact on testability**:
   - Impossible to inject mocks for unit tests
   - Tests limited to integration tests with the real database

## Decision

**We decide to defer implementation of abstract interfaces and dependency injection for now.**

### Specific decisions

1. **No abstract interfaces for Database**: Do not create `IDatabase` until there is a real need to support DBMSs other than SQLite.

2. **No abstract interfaces for Repositories**: Do not create `ITaskRepository`, `IPhaseRepository`, etc. Concrete repositories are sufficient for current needs.

3. **No abstract interfaces for external dependencies**: Do not create interfaces for `httplib::Server` or `cxxopts::Options` until there is a real need to change libraries.

4. **No dependency injection**: Do not implement dependency injection in `main.cpp`, `web.cpp`, `mcp.cpp` for now.

## Justification

### YAGNI principle (You Aren't Gonna Need It)

> "The best code is no code at all. But if you must write code, write the simplest code that works. You aren't gonna need those fancy abstractions until you actually need them."  
> — Free adaptation of Jeff Atwood and Ron Jeffries

- **No immediate real need**: No current need to support multiple DBMSs, change HTTP library, or use mocks in tests.

- **Functional tests**: Existing tests already use in-memory SQLite (`:memory:`) and work correctly. Current integration tests are sufficient to validate system behaviour.

- **Complexity vs benefit**: Implementing abstract interfaces and dependency injection would add:
  - ~15–20 new files (interfaces + implementations)
  - Additional complexity in construction and initialisation
  - Additional maintenance
  - With no immediate real benefit

- **Consistency**: This decision is consistent with the earlier decision not to implement the Strategy pattern for formatters (implicit ADR in the SOLID analysis).

### Current state of the code

The code already respects several SOLID principles:
- **SRP**: Responsibilities well separated (repositories, services, formatters, parsers)
- **OCP**: Extension possible via the Command pattern for new commands
- **DIP**: Direct dependencies on implementations, but acceptable for now

## Consequences

### Positive

- **Simplicity**: Code simpler to understand and maintain
- **Fewer files**: Less complexity in project structure
- **Faster development**: No time spent on unnecessary abstractions
- **Functional tests**: Integration tests with in-memory SQLite work correctly

### Negative

- **Tight coupling**: Code remains coupled to SQLite, httplib, and cxxopts
- **Limited testability**: No possibility to use mocks for isolated unit tests
- **Reduced flexibility**: Changing libraries would require significant refactoring
- **Partial DIP violation**: The Dependency Inversion Principle is not fully respected

### Mitigations

- **Integration tests**: Tests use in-memory SQLite, enabling fast and isolated tests
- **Modular architecture**: Separation into repositories, services, formatters facilitates future refactoring if needed
- **Future review**: This decision will be revisited if a real need arises (e.g. need to support PostgreSQL, need for mocks for unit tests)

## Alternatives considered

### Alternative 1: Implement all abstract interfaces now

**Pros**:
- Full DIP compliance
- Maximum testability with mocks
- Maximum flexibility to change implementation

**Cons**:
- Significant complexity (~20 new files)
- No immediate real need
- Additional maintenance
- Violation of YAGNI principle

**Decision**: Rejected — too much complexity for hypothetical benefit

### Alternative 2: Implement only critical interfaces

**Pros**:
- Compromise between simplicity and flexibility
- Focus on the most critical dependencies

**Cons**:
- Which interfaces are truly critical?
- Risk of creating interfaces that will never be used
- Inconsistency in architecture

**Decision**: Rejected — hard to determine which interfaces are truly needed without a concrete need

## References

- Full SOLID analysis: `docs/internal/analysis-2026-01-28-solid.md`
- YAGNI principle: [Wikipedia - You Aren't Gonna Need It](https://en.wikipedia.org/wiki/You_aren%27t_gonna_need_it)
- SOLID - Dependency Inversion: [Wikipedia - Dependency Inversion Principle](https://en.wikipedia.org/wiki/Dependency_inversion_principle)

## Notes

This decision may be revisited if:
- A real need arises to support multiple DBMSs
- A real need arises to change HTTP or CLI parsing library
- A real need arises to use mocks for isolated unit tests
- The team decides to invest in better testability

In that case, a new ADR will be created to document the change of decision.
