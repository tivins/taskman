# Taskman

## Compilation

### single-config

```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### Compiler et lancer les tests

```shell
cmake --build build --target tests
cd build && ctest -V && cd ..
```

#### Compiler et exécuter taskman

```shell
cmake --build build --target taskman
```

Exécutable

- `build/taskman` (Linux, macOS)
- `build/taskman.exe` (Windows)

### Multi-config

```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### Compiler et lancer les tests

```shell
cmake --build build --config Release --target tests
cd build && ctest -C Release -V && cd ..
```

#### Compiler et exécuter taskman

```shell
cmake --build build --config Release --target taskman
```

Exécutable

- `build/Release/taskman` (Linux, macOS)
- `build/Release/taskman.exe` (Windows)
