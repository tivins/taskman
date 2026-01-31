# Changelog (users)

User-facing changes: new commands, options, formats, and behavior.

## [0.30.0] - 2026-01-31

- **task:note:list-by-ids** : récupération d'une liste de notes par liste d'UID (`--ids id1,id2,...`). Permet d'exploiter les `note_ids` de `task:get` en un seul appel. Commande CLI et outil MCP `taskman_task_note_list_by_ids`.

## [0.29.0] - 2026-01-31

- **task:get** : la sortie inclut désormais la liste des UID des notes liées à la tâche (`note_ids`). En JSON : tableau ; en `--format text` : ligne `note_ids: id1,id2,...`. Permet d'afficher une tâche avec ses notes sans appel séparé.

## [0.28.2] - 2026-01-30

- **Règles** : nouvelle règle `design-patterns` (utilisation des design patterns quand justifié). Disponible via `taskman rules:generate` avec les autres règles (SOLID, RAII, commit-message).

## [0.28.1] - 2026-01-29

- **taskman project:init** : le chemin de l'exécutable est détecté automatiquement ; plus besoin de `--executable` pour générer `.cursor/mcp.json`. Une seule commande `/path/to/taskman project:init` suffit.
- **Windows** : correction quand le chemin de taskman dépasse 260 caractères (mcp:config n'était pas exécuté). Désormais un buffer plus grand est utilisé et mcp.json est bien généré.

## [0.28.0] - 2026-01-29

- **taskman project:init** [--executable \<path\>]: bootstrap d'un nouveau projet. Exécute dans l'ordre : `mcp:config` (si `--executable` fourni), `init`, `rules:generate`, `agents:generate`. Puis recharger Cursor pour utiliser Taskman via l'agent.
- **Outils MCP** : `taskman_rules_generate`, `taskman_agents_generate` (option `output`), et `taskman_project_init` (option `executable`). L'agent peut désormais « générer les règles » ou « récupérer les agents » via MCP.
- **Documentation** : démarrage rapide en 4 étapes (README, usage_mcp.md, usage.md) : télécharger taskman → `/path/to/taskman project:init` → recharger Cursor → utiliser Taskman via l'agent.

## [0.27.0] - 2026-01-29

- **taskman rules:generate** [--output <dir>]: génère les fichiers rules Cursor dans `.cursor/rules/` (ou le répertoire spécifié). Les fichiers sont extraits depuis les assets intégrés dans le binaire (sources dans `embed/rules/*.mdc`). Utile pour synchroniser les règles de code (ex. SOLID, RAII) avec le répertoire Cursor, de la même manière que `agents:generate` pour les agents.

## [0.18.0] - 2026-01-27

- **Notes sur les tâches** : `task:note:add <task-id> --content "..." [--kind completion|progress|issue] [--role <role>]` ajoute une note à une tâche ; `task:note:list <task-id>` liste les notes (ordre par date). Les outils MCP `taskman_task_note_add` et `taskman_task_note_list` exposent les mêmes actions. À l'ajout d'une note, la date de mise à jour (`updated_at`) de la tâche est rafraîchie.
- **demo:generate** : la base démo contient désormais 7 notes (résumés de fin, avancement) sur plusieurs tâches.

## [0.17.1] - 2026-01-27

- **Build** : la compilation sous MSVC/Windows fonctionne à nouveau (assets web embarqués découpés pour rester sous la limite du compilateur).

## [0.17.0] - 2026-01-27

- **created_at / updated_at** : chaque phase, milestone et tâche affiche désormais une date de création (`created_at`) et une date de dernière modification (`updated_at`). Ces champs sont remplis et mis à jour automatiquement (non modifiables). Ils apparaissent en CLI (JSON et `--format text`), dans l'API web (GET /phases, /milestones, /tasks, etc.) et dans les réponses des outils MCP.

## [0.15.0] - 2026-01-27

- **taskman demo:generate**: commande intégrée pour créer une base de données de démonstration remplie avec un exemple réaliste (projet MVP de site e-commerce). Remplace le script Python `scripts/create_demo.py` et permet d'utiliser directement les fonctions du codebase C++ sans avoir à fournir le chemin de l'exécutable. La commande supprime automatiquement la base de données existante (et ses fichiers journal) avant de créer la nouvelle, garantissant un résultat reproductible. Crée 4 phases (Design, Development, Acceptance, Delivery), 4 milestones, 9 tâches avec priorités (sort_order), et leurs dépendances. Disponible aussi comme outil MCP `taskman_demo_generate`.

## [0.14.0] - 2026-01-27

- **taskman mcp:config** [--executable <path>] [--output <file>]: génère ou met à jour automatiquement le fichier `.cursor/mcp.json` avec la configuration du serveur MCP taskman. Prend en paramètre le chemin absolu de l'exécutable taskman. Configure automatiquement `TASKMAN_DB_NAME` avec le répertoire courant + "project_tasks.db" et `TASKMAN_JOURNAL_MEMORY=1`. Fusionne avec les serveurs MCP existants dans le fichier. Utile pour configurer rapidement taskman comme serveur MCP dans Cursor.

## [0.13.0] - 2026-01-27

- **taskman agents:generate** [--output <dir>]: génère les fichiers agents Cursor dans `.cursor/agents/` (ou le répertoire spécifié). Les fichiers sont extraits depuis les assets intégrés dans le binaire. Utile pour configurer les agents de rôles taskman dans Cursor.

## [0.12.0] - 2026-01-25

- **taskman mcp** : mode serveur MCP (Model Context Protocol) sur stdio. Lit les requêtes JSON-RPC sur stdin, écrit les réponses sur stdout. Implémente le protocole MCP 2025-11-25 avec les méthodes `initialize`, `notifications/initialized`, `tools/list`, `tools/call`, et `ping`. Expose les 14 commandes CLI comme outils MCP (`taskman_init`, `taskman_phase_*`, `taskman_milestone_*`, `taskman_task_*`, `taskman_task_dep_*`, `taskman_demo_generate`). À configurer comme serveur MCP dans Cursor (commande `taskman`, args `mcp`). Voir [usage_mcp.md](usage_mcp.md) pour la configuration.

## [0.11.1] - 2026-01-25

- **Vue liste (web)** : à côté du Statut, affichage de « (blocked) » si la tâche dépend d'une tâche non-`done` (utilisation de `/task_deps` et `/task/:id`).

## [0.11.0] - 2026-01-25

- **API web** : GET /milestone/:id, GET /milestones (limit, page), GET /phase/:id, GET /phases (limit, page), GET /task_deps (limit, page, ?task_id=), GET /task/:id/deps.

## [0.10.0] - 2026-01-25

- **taskman web** [--host \<addr\>] [--port \<n\>] : démarre un serveur HTTP (défaut http://127.0.0.1:8080). La page d'accueil charge une interface qui consulte les tâches via l'API JSON (GET /tasks, GET /task/\<id\>).

## [0.9.6] - 2026-01-25

- **Cursor agent / "disk I/O error"**: set `TASKMAN_JOURNAL_MEMORY=1` when running taskman from Cursor's agent (or ensure `CURSOR_AGENT` is set) to avoid SQLite "disk I/O error" in sandboxed environments. See [build.md](build.md).

## [0.9.5] - 2026-01-25

- **taskman \<command\> --help** (or **-h**): detailed help for each command (options, required/optional args). The main help (`taskman -h`) indicates: *Use 'taskman \<command\> --help' for command-specific options.*

## [0.9.3] - 2026-01-25

- **Options**: `-v`, `--version` — show version and exit.

## [0.9.2] - 2026-01-25

- This user changelog; README reorganized so Users and Developers find their docs quickly.

## [0.9.1] - 2026-01-25

- User guide: [usage_cli.md](usage_cli.md), [usage_web.md](usage_web.md), [usage_mcp.md](usage_mcp.md).

## [0.9.0] - 2026-01-25

- GitHub Actions CI: release artifacts for Linux, macOS, Windows. No impact on CLI usage.

## [0.8.0] - 2026-01-25

- **Formats**: JSON for phases, milestones, tasks; `--format text` for `task:add`, `task:get`, `task:list`.
- **Exit codes**: 0 = success, 1 = error (parsing, database, validation).

## [0.7.0] - 2026-01-25

- `task:dep:add <task-id> <dep-id>` and `task:dep:remove <task-id> <dep-id>`. Both tasks must exist; no self-dependency or duplicates.

## [0.6.0] - 2026-01-25

- **Tasks**: `task:add` (--title, --phase, --description, --role, --milestone, --format), `task:get`, `task:list` (--phase, --status, --role, --format), `task:edit`. Task ID is UUID v4.
- **Roles**: project-manager, project-designer, software-architect, developer, summary-writer, documentation-writer.
- **Statuses**: to_do, in_progress, done.

## [0.5.0] - 2026-01-25

- **Milestones**: `milestone:add` (--id, --phase, --name, --criterion, --reached), `milestone:edit`, `milestone:list [--phase]`.
- [build.md](build.md): troubleshooting for "disk I/O error" and `.db-journal` files.

## [0.3.0] - 2026-01-25

- **Phases**: `phase:add` (--id, --name, --status, --sort-order), `phase:edit`, `phase:list` (JSON).

## [0.2.0] - 2026-01-25

- `taskman init` to create the database. Environment variable `TASKMAN_DB_NAME` (default `project_tasks.db`).
