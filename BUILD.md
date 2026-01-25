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

## Dépannage

### « disk I/O error » et fichier `-journal`

Si un `project_tasks.db-journal` (ou `*.db-journal`) est présent, une précédente exécution a été interrompue avant la fermeture propre de la base. À l’ouverture, SQLite tente une récupération ; si une erreur « disk I/O error » apparaît (antivirus, sandbox, système de fichiers), :

1. Arrêter tout processus `taskman` utilisant cette base.
2. Supprimer le fichier `.db` et le `.db-journal` concernés.
3. Relancer `taskman init` pour recréer une base vide.
