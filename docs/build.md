# Taskman

## Download

Get from Github and go inside taskman root directory:

```shel
git clone git@github.com:tivins/taskman.git
cd taskman
````


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

Pour l'interface web : `taskman web` (défaut http://127.0.0.1:8080, voir USAGE.md).

### Multi-config

```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### Build and run tests

```shell
cmake --build build --config Release --target tests
cd build ; ctest -C Release -V ; cd ..
```

#### Build and run taskman

```shell
cmake --build build --config Release --target taskman
```

Executable

- `build/Release/taskman` (Linux, macOS)
- `build/Release/taskman.exe` (Windows)

## Troubleshooting

### "disk I/O error" when using taskman from Cursor's agent

Cursor's agent runs in a **sandbox** that can block SQLite from creating or writing the **rollback journal file** (`-journal`) next to the database. The main `.db` may be writable, but the `-journal` file is not, which leads to `disk I/O error`.

**Fix:** use an in-memory journal so SQLite does not create a `-journal` file on disk:

- **Option A** – before each `taskman` call (recommended for the agent):

  - **PowerShell:** `$env:TASKMAN_JOURNAL_MEMORY="1"; taskman phase:list`
  - **Bash:** `TASKMAN_JOURNAL_MEMORY=1 taskman phase:list`

- **Option B** – in your project's agent rules (e.g. `.cursor/rules` or instructions): tell the agent to **always set `TASKMAN_JOURNAL_MEMORY=1`** (and `TASKMAN_DB_NAME` if needed) when running `taskman`.

- **Option C** – if Cursor sets `CURSOR_AGENT` in the environment, taskman will automatically use `PRAGMA journal_mode=MEMORY`.

With `journal_mode=MEMORY`, the journal is kept in RAM. Durability is slightly lower if the process is killed during a write; for normal agent usage this is usually acceptable. The `.db` file itself is still written to disk.
