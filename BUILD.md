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
