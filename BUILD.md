# Taskman

## Build

### single-config

```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### Build and run tests

```shell
cmake --build build --target tests
cd build && ctest -V && cd ..
```

#### Build and run taskman

```shell
cmake --build build --target taskman
```

Executable

- `build/taskman` (Linux, macOS)
- `build/taskman.exe` (Windows)

### Multi-config

```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### Build and run tests

```shell
cmake --build build --config Release --target tests
cd build && ctest -C Release -V && cd ..
```

#### Build and run taskman

```shell
cmake --build build --config Release --target taskman
```

Executable

- `build/Release/taskman` (Linux, macOS)
- `build/Release/taskman.exe` (Windows)

## Troubleshooting

### "disk I/O error" and `-journal` file

If a `project_tasks.db-journal` (or `*.db-journal`) file is present, a previous run was interrupted before the database was cleanly closed. On open, SQLite attempts recovery; if a "disk I/O error" occurs (antivirus, sandbox, filesystem):

1. Stop any `taskman` process using this database.
2. Delete the `.db` and `.db-journal` files in question.
3. Run `taskman init` again to recreate an empty database.

### "disk I/O error" when using taskman from Cursor’s agent

Cursor’s agent runs in a **sandbox** that can block SQLite from creating or writing the **rollback journal file** (`-journal`) next to the database. The main `.db` may be writable, but the `-journal` file is not, which leads to `disk I/O error`.

**Fix:** use an in-memory journal so SQLite does not create a `-journal` file on disk:

- **Option A** – before each `taskman` call (recommended for the agent):

  - **PowerShell:** `$env:TASKMAN_JOURNAL_MEMORY="1"; taskman phase:list`
  - **Bash:** `TASKMAN_JOURNAL_MEMORY=1 taskman phase:list`

- **Option B** – in your project’s agent rules (e.g. `.cursor/rules` or instructions): tell the agent to **always set `TASKMAN_JOURNAL_MEMORY=1`** (and `TASKMAN_DB_NAME` if needed) when running `taskman`.

- **Option C** – if Cursor sets `CURSOR_AGENT` in the environment, taskman will automatically use `PRAGMA journal_mode=MEMORY`.

With `journal_mode=MEMORY`, the journal is kept in RAM. Durability is slightly lower if the process is killed during a write; for normal agent usage this is usually acceptable. The `.db` file itself is still written to disk.
