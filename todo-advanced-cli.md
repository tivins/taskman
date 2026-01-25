# Todo — CLI avancée (version, help, help sous-commandes)

## Objectif

Ajouter la gestion des paramètres suivants :

- `taskman -v|--version`
- `taskman -h|--help`
- `taskman -h|--help <subcommand>` ou `taskman <subcommand> -h|--help`

---

## Convention retenue

La forme **standard** pour l’aide d’une sous-commande est :

- **`taskman <subcommand> -h|--help`** — les options -h/--help sont placées **après** la sous-commande.

On peut en plus accepter **`taskman -h <subcommand>`** comme variante : quand `argv[1]` vaut `-h`/`--help` et qu’il y a un `argv[2]`, on redispatch vers la sous-commande en passant `-h` (équivalent à `taskman <subcommand> -h`).

---

## Modifications à faire

### 1. `main.cpp` — `-v`, `-h` et redispatch `-h <subcommand>`

Traiter **avant** le dispatch actuel :

- **`taskman -v` / `taskman --version`**  
  Si `argv[1]` est `-v` ou `--version` → afficher la version et `return 0`. La version vient de `TASKMAN_VERSION` (voir CMake).

- **`taskman -h` / `taskman --help`** (sans autre argument)  
  Si `argv[1]` est `-h` ou `--help` et `argc == 2` → `print_usage(argv[0])` et `return 0`.

- **`taskman -h <subcommand>` / `taskman --help <subcommand>`**  
  Si `argv[1]` est `-h` ou `--help` et `argc >= 3` :
  - Récupérer la sous-commande dans `argv[2]`.
  - Construire un `argv'` équivalent à `taskman <subcommand> -h`, ex. `argc' = 2`, `argv' = [ "<subcommand>", "-h" ]`.
  - Ouvrir la DB (comme pour les autres commandes), appeler le `cmd_*` correspondant avec `(argc', argv', db)`.
  - Chaque `cmd_*` doit gérer `-h`/`--help` via cxxopts et, si `result.count("help")`, afficher `opts.help()` et sortir sans accès DB. Ainsi `taskman -h phase:add` et `taskman phase:add -h` donnent le même résultat.

Ordre dans `main` :

1. `argc < 2` → `print_usage` + `return 1` (inchangé).
2. Si `argv[1]` est `-v` ou `--version` → afficher version + `return 0`.
3. Si `argv[1]` est `-h` ou `--help` :
   - `argc == 2` → `print_usage` + `return 0`.
   - `argc >= 3` → redispatch vers la sous-commande `argv[2]` avec `argv' = [ argv[2], "-h" ]`, `argc' = 2`.
4. Sinon, dispatch habituel par `argv[1]`.

Pour le redispatch `-h <subcommand>`, factoriser le dispatch (fonction `dispatch(cmd, argc, argv, db)` ou table de correspondance) pour éviter la duplication.

---

### 2. Sous-commandes (phase, milestone, task) — `-h|--help` dans cxxopts

cxxopts n’ajoute pas `-h`/`--help` par défaut. Pour chaque `cmd_*` utilisant `cxxopts::Options` :

1. Dans `add_options()` : ajouter  
   `("h,help", "Show help")`  
   ou  
   `("h,help", "Show help", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))`.

2. Après `opts.parse(...)` :  
   `if (result.count("help")) { std::cout << opts.help() << "\n"; return 0; }`  
   **avant** toute utilisation de la DB.

Fichiers et commandes concernés :

- **phase.cpp** : `cmd_phase_add`, `cmd_phase_edit`, `cmd_phase_list`
- **milestone.cpp** : `cmd_milestone_add`, `cmd_milestone_edit`, `cmd_milestone_list`
- **task.cpp** : `cmd_task_add`, `cmd_task_get`, `cmd_task_list`, `cmd_task_edit`

Pour `phase:list` et `milestone:list` (peu ou pas d’options cxxopts) : soit ajouter un `cxxopts::Options` minimal avec `h,help`, soit une détection manuelle de `-h`/`--help` dans `argv` au début.

---

### 3. Stubs `task:dep:add` et `task:dep:remove`

Pour l’instant ils affichent « not implemented ». Pour `taskman task:dep:add -h` (et `taskman -h task:dep:add` si supporté) :

- **Option A** : au début du bloc, si `argc >= 2` et `argv[1]` est `-h` ou `--help`, afficher un court texte du type  
  `taskman task:dep:add <task_id> <dep_id>  (not implemented yet)`  
  et `return 0`.
- **Option B** : laisser tel quel et ajouter `h,help` lors de l’implémentation réelle.

---

### 4. Version — `TASKMAN_VERSION` depuis CMake

La version est déjà dans CMake : `project(taskman VERSION 0.6.0 ...)`.

- **CMakeLists.txt** :  
  `target_compile_definitions(taskman PRIVATE TASKMAN_VERSION="${PROJECT_VERSION}")`

- **main.cpp** :  
  - `#ifdef TASKMAN_VERSION` … `#define TASKMAN_VERSION_STR` ou utiliser directement la macro.
  - Dans le bloc `-v`/`--version` :  
    `std::cout << "taskman " TASKMAN_VERSION "\n";`  
  - `#else` éventuel avec `"unknown"` pour les builds sans cette définition.

---

## Récapitulatif des points de code

| Fichier / zone | Rôle |
|----------------|------|
| **main.cpp** | Gestion de `-v`/`--version` ; `-h`/`--help` (global) ; redispatch `-h`/`--help` + sous-commande vers le `cmd_*` avec `-h`. |
| **CMakeLists.txt** | `target_compile_definitions(taskman PRIVATE TASKMAN_VERSION="${PROJECT_VERSION}")`. |
| **phase.cpp** | Option `h,help` dans cxxopts et `if (result.count("help")) { cout << opts.help(); return 0; }` dans `cmd_phase_add`, `cmd_phase_edit` ; pour `cmd_phase_list`, cxxopts minimal ou test manuel de `-h`/`--help`. |
| **milestone.cpp** | Idem pour `cmd_milestone_add`, `cmd_milestone_edit`, `cmd_milestone_list`. |
| **task.cpp** | Idem pour `cmd_task_add`, `cmd_task_get`, `cmd_task_list`, `cmd_task_edit`. |
| **Stubs task:dep:\*** | Détection de `-h`/`--help` et message d’aide minimal, ou report à l’implémentation. |

---

## Exemple d’option help avec cxxopts

```cpp
opts.add_options()
    ("h,help", "Show help")
    // ... autres options
```

Puis :

```cpp
result = opts.parse(argc, argv);
if (result.count("help")) {
    std::cout << opts.help() << "\n";
    return 0;
}
```

Ou en booléen :

```cpp
("h,help", "Show help", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
```

---

## Ordre de mise en œuvre suggéré

1. **CMake** : définir `TASKMAN_VERSION` pour l’exécutable `taskman`.
2. **main.cpp** :  
   - `-v`/`--version` ;  
   - `-h`/`--help` seul ;  
   - (optionnel) `-h`/`--help` + sous-commande avec redispatch.
3. **Une sous-commande** (ex. `cmd_milestone_add`) : ajout de `h,help` et `if (result.count("help"))` pour valider le flux.
4. **Généraliser** à toutes les `cmd_*` (phase, milestone, task) et aux stubs si besoin.
