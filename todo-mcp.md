# Todo — MCP (Model Context Protocol) directement dans le C++

## Objectif

Exposer taskman comme **serveur MCP** en ajoutant un mode `taskman mcp` : le binaire lit les requêtes JSON-RPC sur stdin et écrit les réponses sur stdout. Les commandes existantes (init, phase:*, milestone:*, task:*, task:dep:*) sont exposées comme **outils** MCP ; chaque appel `tools/call` se traduit en exécution de la `cmd_*` correspondante. Transport **stdio** uniquement (pas de HTTP dans un premier temps).

---

## TODO liste (implémentation)

### Infra et squelette

- [x] **CMake** — Ajouter `src/mcp.cpp` à `add_executable(taskman ...)`.
- [x] **`src/mcp.hpp`** — Déclarer `int run_mcp_server();` (évent. `struct McpTool` ou config outils).
- [x] **`src/mcp.cpp`** — Boucle stdio : `while (getline(cin, line))`, parse `nlohmann::json::parse(line)`.
- [x] **`src/mcp.cpp`** — Détecter `method` / `id`, ignorer les lignes vides ou non-JSON (ou erreur -32700).
- [x] **`src/mcp.cpp`** — Envoyer réponses : `cout << response.dump() << "\n"; cout.flush();`.
- [x] **`src/mcp.cpp`** — Erreurs de parse → JSON-RPC `-32700` (Parse error). Requête invalide → `-32600` (Invalid Request).
- [x] **`main.cpp`** — Branche `if (strcmp(cmd, "mcp") == 0) return run_mcp_server();`.
- [x] **`main.cpp`** — Ajouter `{ "mcp", "MCP server (stdio): read JSON-RPC on stdin, write on stdout" }` dans `COMMANDS` et usage.

### Lifecycle

- [x] **`initialize`** — Répondre `protocolVersion`, `capabilities: { tools: { listChanged: false } }`, `serverInfo: { name: "taskman", version }`. Réutiliser `TASKMAN_VERSION`. Réponse avec `id` reçu.
- [x] **`initialize`** — Si `protocolVersion` non supportée (ex. pas `2025-11-25`) → erreur `-32602` "Unsupported protocol version".
- [x] **`notifications/initialized`** — Accepter sans répondre (notification), marquer phase « opération ».

### Méthodes

- [ ] **`tools/list`** — Réponse `{ "tools": [ ... ] }` à partir d’une table statique (nom, description, inputSchema) pour les 13 outils.
- [ ] **`tools/call`** — Extraire `params.name` et `params.arguments` ; si `name` inconnu → `-32602` "Unknown tool: <name>". Si `arguments` absent ou non-objet → `-32602` "Invalid arguments".
- [ ] **`tools/call`** — Pour un `name` connu : construire `argv` (positionals + `--key` `value`), ouvrir DB (`get_db_path()`), capturer cout/cerr, appeler la `cmd_*`, former `CallToolResult` : `content: [{ type: "text", text }]`, `isError: (exit_code != 0)`.
- [ ] **`ping`** — Répondre `result: {}`.
- [ ] **Méthode inconnue** — Réponse erreur JSON-RPC `-32601` "Method not found".

### Table des 13 outils (nom, description, inputSchema, positionals, options)

- [ ] `taskman_init` — inputSchema `{}`, pas d’argv.
- [ ] `taskman_phase_add` — id, name (requis), status, sort-order. argv : `--id`, `--name`, etc.
- [ ] `taskman_phase_edit` — positionnel `id`, options name, status, sort-order.
- [ ] `taskman_phase_list` — inputSchema `{}`, argv `[]`.
- [ ] `taskman_milestone_add` — id, phase (requis), name, criterion, reached.
- [ ] `taskman_milestone_edit` — positionnel `id`, options name, criterion, reached, phase.
- [ ] `taskman_milestone_list` — option `phase`.
- [ ] `taskman_task_add` — title, phase (requis), description, role, milestone, format.
- [ ] `taskman_task_get` — positionnel `id`, option `format`.
- [ ] `taskman_task_list` — phase, status, role, format.
- [ ] `taskman_task_edit` — positionnel `id`, options title, description, status, role, milestone.
- [ ] `taskman_task_dep_add` — positionnels `task-id`, `dep-id`.
- [ ] `taskman_task_dep_remove` — positionnels `task-id`, `dep-id`.

### Conversion `arguments` → `argv` et capture

- [ ] **Construction argv** — Pour chaque outil : d’abord positionnels (ordre fixe), puis paires `"--<key>"`, `value` pour les options présentes dans `arguments`. Ignorer clés inconnues. Valeur : string ; si number → `to_string`. `reached` : 0/1 → `"0"`/`"1"`.
- [ ] **`argv` vers `cmd_*`** — `std::vector<std::string> argv_strs` ; `std::vector<char*> argv_ptrs` pointant sur `s.data()` (ou `&s[0]`) ; passer `argv_ptrs.size()`, `argv_ptrs.data()` à la `cmd_*`. Ne pas ajouter de chaîne vide.
- [ ] **Capture cout/cerr** — Rdbuf de `cout`/`cerr` sauvegardé, remplacé par `ostringstream` ; appel `cmd_*` ; restauration ; en cas d’exception, restaurer les rdbuf (RAII ou try/finally).
- [ ] **Dispatch name → cmd_*** — Table ou `if/else` : `taskman_init` → ouvrir DB puis `init_schema(db)` (pas de `cmd_init`), `taskman_phase_list` → `cmd_phase_list`, etc. Pour `taskman_init` : pas d’argv ; succès → `content.text` = `"Database initialized."` ou vide ; échec → stderr capturé, `isError: true`.
- [ ] **`argc`/`argv` pour cmd_*** — Pour `cmd_phase_list` (aucun arg) : `argc=0` et `argv=nullptr` ou `argc=1` avec `argv[0]` factice selon robustesse. Pour les autres : `argv` construit à partir de `arguments`.

### Intégration et tests

- [ ] **Tests manuels** — `echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{...}}' | taskman mcp` → réponse `InitializeResult`.
- [ ] **Tests manuels** — `tools/list` → 13 outils.
- [ ] **Tests manuels** — `tools/call` pour `taskman_init`, `taskman_phase_list`, `taskman_phase_add`, `taskman_task_add`, `taskman_task_list`, `taskman_task_get`, `taskman_task_edit`, `taskman_task_dep_add`, `taskman_task_dep_remove`, `taskman_milestone_*`.
- [ ] **Tests manuels** — `tools/call` avec task/phase inexistante → `isError: true` et message d’erreur dans `content`.
- [ ] **Documentation** — Section « MCP » dans README ou USAGE : `taskman mcp`, config Cursor (commande, args, env `TASKMAN_DB_NAME`, `TASKMAN_JOURNAL_MEMORY`).

---

## 1. Références

- [MCP Specification (2025-11-25)](https://modelcontextprotocol.io/specification/2025-11-25)
- [Lifecycle](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle)
- [Transports (stdio)](https://modelcontextprotocol.io/specification/2025-11-25/basic/transports#stdio)
- [Tools (list, call)](https://modelcontextprotocol.io/specification/2025-11-25/server/tools)

---

## 2. Transport stdio

- Le client (Cursor) lance `taskman mcp` en **sous-processus**.
- Le serveur **lit** les messages JSON-RPC sur **stdin** (une requête par ligne ; les messages sont délimités par newline, sans newline imbriqué dans le JSON).
- Le serveur **écrit** les réponses JSON-RPC sur **stdout** uniquement. Tout log / debug va sur **stderr**.
- Rien d’autre que des messages MCP valides sur stdout.

Format d’une ligne : objet JSON complet, terminé par `\n`. Encodage UTF-8.

---

## 3. Lifecycle MCP

### 3.1 `initialize` (requête)

Le client envoie en premier une requête `initialize` avec `protocolVersion`, `capabilities`, `clientInfo`.

Le serveur **doit** répondre par une **réponse** (pas une notification) du type :

```json
{
  "jsonrpc": "2.0",
  "id": <id reçu>,
  "result": {
    "protocolVersion": "2025-11-25",
    "capabilities": {
      "tools": { "listChanged": false }
    },
    "serverInfo": {
      "name": "taskman",
      "version": "<TASKMAN_VERSION>"
    }
  }
}
```

- `protocolVersion` : on accepte `2025-11-25` (et éventuellement `2024-11-05` si on veut être permissif ; sinon renvoyer une erreur si version non supportée).
- `capabilities` : au minimum `tools: { listChanged: false }`. Pas de `resources`, ni `prompts` dans un premier temps.
- `serverInfo.name` = `"taskman"`, `serverInfo.version` = `TASKMAN_VERSION` (déjà défini en CMake / `version.h`).

En cas de version non supportée : erreur JSON-RPC, ex. `code: -32602`, `message: "Unsupported protocol version"`.

### 3.2 `notifications/initialized` (notification)

Après la réponse à `initialize`, le client envoie la notification `notifications/initialized`. Aucune réponse attendue. Le serveur la reçoit et peut considérer que la phase d’opération a commencé.

### 3.3 Phase d’opération

Le serveur doit alors traiter notamment :

- `tools/list`
- `tools/call`
- `ping` (optionnel mais recommandé : réponses `{}`)

Toute autre méthode peut être renvoyée en erreur « méthode inconnue » (`-32601`) ou ignorée selon la spec.

---

## 4. Méthodes à implémenter

### 4.1 `tools/list`

**Requête :**

```json
{ "jsonrpc": "2.0", "id": 2, "method": "tools/list", "params": {} }
```

`params` peut contenir `cursor` pour pagination ; on peut ignorer la pagination au départ et tout renvoyer.

**Réponse :**

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "tools": [
      {
        "name": "taskman_init",
        "description": "Create and initialize the database tables (phases, milestones, tasks, task_deps). Run once when starting a new project.",
        "inputSchema": { "type": "object", "properties": {}, "additionalProperties": false }
      },
      ... (un objet par outil, voir section 5)
    ]
  }
}
```

Les noms d’outils doivent être uniques, 1–128 caractères, [a-zA-Z0-9_-.] (pas d’espace). On utilisera le préfixe `taskman_` et le mapping suivant :

| Commande CLI     | Nom d’outil MCP      |
|------------------|----------------------|
| `init`           | `taskman_init`       |
| `phase:add`      | `taskman_phase_add`  |
| `phase:edit`     | `taskman_phase_edit` |
| `phase:list`     | `taskman_phase_list` |
| `milestone:add`  | `taskman_milestone_add`  |
| `milestone:edit` | `taskman_milestone_edit` |
| `milestone:list` | `taskman_milestone_list` |
| `task:add`       | `taskman_task_add`    |
| `task:get`       | `taskman_task_get`    |
| `task:list`      | `taskman_task_list`   |
| `task:edit`      | `taskman_task_edit`   |
| `task:dep:add`   | `taskman_task_dep_add`   |
| `task:dep:remove`| `taskman_task_dep_remove` |

La commande `web` n’est pas exposée comme outil MCP (serveur HTTP long-running, incompatible avec une boucle stdio synchrone).

### 4.2 `tools/call`

**Requête :**

```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "tools/call",
  "params": {
    "name": "taskman_task_list",
    "arguments": { "phase": "P1", "format": "json" }
  }
}
```

**Réponse en succès :**

```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "content": [{ "type": "text", "text": "<sortie de la commande (stdout)>" }],
    "isError": false
  }
}
```

**Réponse en erreur d’exécution (métier) :**

Par ex. tâche introuvable, phase invalide, dépendance circulaire, etc. On renvoie le message d’erreur (actuellement sur stderr) dans `content` et `isError: true` :

```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "content": [{ "type": "text", "text": "taskman: task not found: xyz" }],
    "isError": true
  }
}
```

Pour les erreurs de **protocole** (outil inconnu, `arguments` invalides au sens MCP) : utiliser une **erreur JSON-RPC** (`error` avec `code` / `message`), pas un `result` avec `isError`.

---

## 5. Définition des outils (nom, description, inputSchema)

Pour chaque outil, on stocke en interne :

- `name`
- `description` (courte, pour l’LLM)
- `inputSchema` (JSON Schema, `type: "object"`, `properties`, `required` si besoin)
- Pour la conversion `arguments` → `argv` : liste ordonnée des **paramètres positionnels** (noms des clés JSON), puis les noms des **options nommées** (équivalent `--key`). Les noms dans `arguments` correspondent aux noms des options CLI sans `--` (ex. `task-id`, `dep-id`, `sort-order`).

Conventions :

- Les clés dans `arguments` reprennent les noms d’options CLI : `sort-order`, `task-id`, `dep-id`, `phase`, `status`, `format`, etc.
- Pour les positionnels : `id` (phase:edit, milestone:edit, task:get, task:edit), `task-id` et `dep-id` (task:dep:add, task:dep:remove).

### 5.1 `taskman_init`

- **inputSchema :**  
  `{ "type": "object", "properties": {}, "additionalProperties": false }`
- **Positionnels :** aucun  
- **Options :** aucune

### 5.2 `taskman_phase_add`

- **Options :** `id` (requis), `name` (requis), `status`, `sort-order`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "id": { "type": "string", "description": "Unique phase identifier" },
    "name": { "type": "string", "description": "Phase name" },
    "status": { "type": "string", "enum": ["to_do", "in_progress", "done"], "description": "Phase status" },
    "sort-order": { "type": ["string", "integer"], "description": "Display order (integer)" }
  },
  "required": ["id", "name"]
}
```

- **Construction argv :** `["--id", v_id, "--name", v_name]` + si présents `["--status", v_status]`, `["--sort-order", v_sort_order]`. Pour `sort-order`, convertir entier en string.

### 5.3 `taskman_phase_edit`

- **Positionnels :** `id`
- **Options :** `name`, `status`, `sort-order`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "id": { "type": "string", "description": "Phase ID to edit" },
    "name": { "type": "string" },
    "status": { "type": "string", "enum": ["to_do", "in_progress", "done"] },
    "sort-order": { "type": ["string", "integer"] }
  },
  "required": ["id"]
}
```

- **Construction argv :** `[id]` puis `--name`, `--status`, `--sort-order` si présents. Au moins une option requise pour que la commande fasse un effet ; sinon la cmd_* actuelle sort sans rien faire (code 0). On peut laisser ce comportement et renvoyer le stdout (vide) comme succès.

### 5.4 `taskman_phase_list`

- **inputSchema :**  
  `{ "type": "object", "properties": {}, "additionalProperties": false }`
- **argv :** `[]`

### 5.5 `taskman_milestone_add`

- **Options :** `id`, `phase` (requis), `name`, `criterion`, `reached`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "id": { "type": "string", "description": "Unique milestone identifier" },
    "phase": { "type": "string", "description": "Parent phase ID" },
    "name": { "type": "string" },
    "criterion": { "type": "string" },
    "reached": { "type": "string", "enum": ["0", "1"], "description": "0=not reached, 1=reached" }
  },
  "required": ["id", "phase"]
}
```

- **argv :** `--id`, `--phase`, puis optionnels `--name`, `--criterion`, `--reached`. `reached` en string `"0"` ou `"1"`.

### 5.6 `taskman_milestone_edit`

- **Positionnels :** `id`
- **Options :** `name`, `criterion`, `reached`, `phase`
- **inputSchema :**  
  Même principe que phase:edit, avec `id` requis et les 4 autres en option. `reached` : `["0","1"]`.

### 5.7 `taskman_milestone_list`

- **Options :** `phase`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "phase": { "type": "string", "description": "Filter by phase ID" }
  }
}
```

### 5.8 `taskman_task_add`

- **Options :** `title`, `phase` (requis), `description`, `role`, `milestone`, `format`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "title": { "type": "string" },
    "phase": { "type": "string" },
    "description": { "type": "string" },
    "role": { "type": "string", "enum": ["project-manager", "project-designer", "software-architect", "developer", "summary-writer", "documentation-writer"] },
    "milestone": { "type": "string" },
    "format": { "type": "string", "enum": ["json", "text"], "default": "json" }
  },
  "required": ["title", "phase"]
}
```

### 5.9 `taskman_task_get`

- **Positionnels :** `id`
- **Options :** `format`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "id": { "type": "string", "description": "Task UUID" },
    "format": { "type": "string", "enum": ["json", "text"] }
  },
  "required": ["id"]
}
```

- **argv :** `[id]` puis éventuellement `["--format", format]`.

### 5.10 `taskman_task_list`

- **Options :** `phase`, `status`, `role`, `format`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "phase": { "type": "string" },
    "status": { "type": "string", "enum": ["to_do", "in_progress", "done"] },
    "role": { "type": "string", "enum": ["project-manager", "project-designer", "software-architect", "developer", "summary-writer", "documentation-writer"] },
    "format": { "type": "string", "enum": ["json", "text"] }
  }
}
```

### 5.11 `taskman_task_edit`

- **Positionnels :** `id`
- **Options :** `title`, `description`, `status`, `role`, `milestone`
- **inputSchema :**  
  `id` requis ; les 5 autres optionnels (enum pour `status` et `role` comme ci‑dessus).

### 5.12 `taskman_task_dep_add`

- **Positionnels :** `task-id`, `dep-id`
- **inputSchema :**

```json
{
  "type": "object",
  "properties": {
    "task-id": { "type": "string", "description": "Task that depends on another" },
    "dep-id": { "type": "string", "description": "Task that must be completed first" }
  },
  "required": ["task-id", "dep-id"]
}
```

- **argv :** `[arguments["task-id"], arguments["dep-id"]]` (cxxopts `parse_positional({"task-id","dep-id"})` attend ces deux chaînes en position 0 et 1).

### 5.13 `taskman_task_dep_remove`

- **Positionnels :** `task-id`, `dep-id`
- **inputSchema :** identique à `taskman_task_dep_add` (sémantique différente, même forme).

---

## 6. Conversion `arguments` (JSON) → `argv` puis appel `cmd_*`

### 6.1 Règles

1. Pour chaque outil, on a une config :  
   - `positional_keys` : `["id"]` ou `["task-id","dep-id"]` ou `[]`
2. Construire `argv` :
   - Pour chaque `k` dans `positional_keys` : si `arguments.contains(k)` et `arguments[k]` non nul, `argv.push_back(arguments[k].as_string())` (pour les entiers, les convertir en string, ex. `sort-order`).
   - Pour les autres propriétés de l’`inputSchema` (sauf celles purement internes) : si `arguments.contains(key)` et valeur non nulle, ajouter `"--" + key`, puis la valeur en string.
3. Rediriger temporairement **stdout** (et éventuellement **stderr**) dans des buffers (ou des `std::ostringstream` / `std::stringstream`), ou exécuter la `cmd_*` dans un flux capturé.  
   - Les `cmd_*` écrivent actuellement dans `std::cout` / `std::cerr`. Pour ne pas modifier les `cmd_*`, on peut :
     - **Option A :** rediriger `stdout` / `stderr` au niveau FILE (dup2, freopen) avant d’appeler la `cmd_*`, puis les restaurer après. Délicat en C++ (globaux).
     - **Option B :** refactorer les `cmd_*` pour accepter des streams en paramètre (ex. `std::ostream& out`, `std::ostream& err`). Plus propre long terme, mais plus de changements.
     - **Option C :** ajouter une couche d’adaptateur : des fonctions `cmd_*_string(int argc, char** argv, Database& db, std::string& out, std::string& err, int& exit_code)` qui font les redirections en interne (sauvegarde de `cout`/`cerr`, remplacement par des ostringstream, appel de `cmd_*`, restauration). Possible sans toucher aux `cmd_*` en les appelant depuis un bloc qui a redirigé cout/cerr.
4. **Option C (recommandée pour garder les `cmd_*` inchangées) :**
   - Sauvegarder les `std::cout.rdbuf()` et `std::cerr.rdbuf()`.
   - Remplacer par les rdbuf de `std::ostringstream` pour `cout` et `cerr`.
   - Appeler `cmd_*`.
   - Restaurer les rdbuf.
   - Si `return` = 0 : `content = { "type": "text", "text": out_str }`, `isError = false`.
   - Si `return` = 1 : `content = { "type": "text", "text": err_str }` (ou `out_str + "\n" + err_str` si on veut tout ; en pratique les `cmd_*` mettent les erreurs sur stderr). `isError = true`.

### 6.2 Gestion des types dans `arguments`

- `arguments` vient du JSON : nombres, booléens, null. Pour les options qui attendent une string (ex. `--sort-order`), faire `std::to_string(v)` si c’est un nombre ; pour les enums, le client devrait envoyer une string. Si on reçoit un entier pour `reached`, accepter 0/1 et convertir en `"0"`/`"1"`.

### 6.3 Ordre des `--option` dans `argv`

L’ordre n’a pas d’importance pour cxxopts. On peut ajouter les options dans un ordre fixe (ex. ordre alphabétique ou ordre de déclaration dans l’inputSchema).

---

## 7. Structure des fichiers et du code

### 7.1 Nouveaux fichiers

- **`src/mcp.hpp`**  
  Déclarations :  
  - `int run_mcp_server();` — point d’entrée du mode MCP. Lit stdin en boucle, parse les lignes JSON-RPC, dispatche selon `method`, écrit les réponses sur stdout. Utilise `get_db_path()` comme aujourd’hui pour ouvrir la DB à chaque `tools/call` (ou une DB gardée ouverte pour toute la session ; pour stdio, une DB par process est cohérent : on peut l’ouvrir au premier `tools/call` et la garder ouverte).
  - Éventuellement : `struct McpTool { std::string name; std::string description; nlohmann::json inputSchema; std::vector<std::string> positional; std::string cmd; };` et `std::vector<McpTool> get_mcp_tools();` pour centraliser les définitions.

- **`src/mcp.cpp`**  
  Implémentation :
  - Boucle stdio : `while (std::getline(std::cin, line))` (ou équivalent pour lire une ligne complète). Gérer le cas où le client ferme stdin (fin de flux) → sortie de la boucle, fin du process.
  - Parse JSON : `nlohmann::json::parse(line)`.
  - Vérifier `jsonrpc == "2.0"` et `method` ou `id` pour les requêtes.
  - `initialize` → construire `InitializeResult` (protocole, capabilities, serverInfo) et envoyer la réponse.
  - `notifications/initialized` → rien à renvoyer.
  - `tools/list` → construire le tableau `tools` à partir de la table interne (nom, description, inputSchema pour chaque outil), renvoyer `{ "tools": [...] }`.
  - `tools/call` → extraire `name` et `arguments`. Trouver l’outil par `name`. Construire `argv` à partir de `arguments` et de la config (positionals + options). Ouvrir la DB (`get_db_path()`). Rediriger cout/cerr, appeler la `cmd_*` correspondante, restaurer. Former `CallToolResult` avec `content` et `isError`. Réponse JSON-RPC `result`.
  - `ping` → si on l’implémente : `result: {}`.
  - Pour toute autre `method` : erreur `-32601` (Method not found).
  - Erreurs de parse JSON : `-32700` (Parse error) ou `-32600` (Invalid Request).
  - Envoi : `std::cout << response.dump() << "\n"; std::cout.flush();`

### 7.2 Modifications de `main.cpp`

- Après le traitement de `-v` / `-h` (et avant le dispatch des autres commandes), ajouter :

  ```cpp
  if (std::strcmp(cmd, "mcp") == 0) {
      return taskman::run_mcp_server();
  }
  ```

- Dans `COMMANDS` (et `get_usage_string`), ajouter :  
  `{ "mcp", "MCP server (stdio): read JSON-RPC on stdin, write on stdout" }`.

### 7.3 Dépendances

- **nlohmann/json** : déjà utilisé (task, phase, milestone, formats). Suffisant pour parser les requêtes et construire les réponses.
- Aucune dépendance MCP externe : tout le protocole (lifecycle, tools/list, tools/call, format des messages) est codé à la main.

### 7.4 Base de données

- Chemin : `get_db_path()` (inchangé, `TASKMAN_DB_NAME` ou défaut `project_tasks.db`).
- Pour `tools/call` : ouvrir `Database` au début de chaque `tools/call` (comme aujourd’hui dans `main`), exécuter la `cmd_*`, fermer ou réutiliser. Pour simplifier, on peut ouvrir la DB une fois au premier `tools/call` ou au `initialize` et la garder ouverte jusqu’à la fin du process (évite de rouvrir à chaque appel). Si on garde une connexion ouverte, attention aux timeouts ou à la fermeture propre en cas de fin de stdin.

---

## 8. Capture de stdout/stderr des `cmd_*`

Les `cmd_*` utilisent `std::cout` et `std::cerr` directement. Pour obtenir leur sortie sans modifier leur signature :

```cpp
#include <sstream>
#include <iostream>

void capture_and_call(int (*cmd)(int, char*[], Database&), int argc, char** argv, Database& db,
                     std::string& out, std::string& err, int& exit_code) {
    std::ostringstream oss_out, oss_err;
    auto old_out = std::cout.rdbuf(oss_out.rdbuf());
    auto old_err = std::cerr.rdbuf(oss_err.rdbuf());
    exit_code = cmd(argc, argv, db);
    std::cout.rdbuf(old_out);
    std::cerr.rdbuf(old_err);
    out = oss_out.str();
    err = oss_err.str();
}
```

En cas d’exception dans `cmd_*`, il faudrait aussi restaurer les rdbuf (RAII ou try/finally). Un scope guard pour `rdbuf` est recommandé.

---

## 9. Table de dispatch `name` → `cmd_*`

Pour chaque `taskman_*`, on doit appeler la bonne `cmd_*` avec un `argv` construit. La signature des `cmd_*` est `int cmd_*(int argc, char* argv[], Database& db)`. On peut :

- construire un `std::vector<std::string>` pour les parties de l’argv, puis un `std::vector<char*>` pointant sur les `c_str()` (ou une plage de `char*` pour `argv`), en prenant garde à la durée de vie des `char*` (les `std::string` doivent rester vivantes pendant l’appel) ;
- ou allouer un `std::vector<char>` pour une copie de type “argv contigu” et bâtir `char** argv` ; plus lourd.

Approche simple : `std::vector<std::string> argv_strs` ; remplir avec les positionnels et les paires `"--key"`, `"value"`. Ensuite :

```cpp
std::vector<char*> argv_ptrs;
for (auto& s : argv_strs) argv_ptrs.push_back(&s[0]);  // C++11: &s[0] ok tant que non empty
cmd_xxx(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
```

En C++17, `s.data()` est préférable ; en C++11, `&s[0]` est valide pour une `std::string` non vide. Pour une chaîne vide, ne pas l’ajouter au `argv` (éviter `""`).

---

## 10. Configuration Cursor pour `taskman mcp`

Exemple de configuration MCP côté Cursor (fichier de config MCP, selon la doc Cursor) :

```json
{
  "mcpServers": {
    "taskman": {
      "command": "taskman",
      "args": ["mcp"],
      "env": {
        "TASKMAN_DB_NAME": "project_tasks.db",
        "TASKMAN_JOURNAL_MEMORY": "1"
      }
    }
  }
}
```

Le chemin de `taskman` doit être résolvable (PATH ou chemin absolu). `TASKMAN_JOURNAL_MEMORY=1` ou `CURSOR_AGENT` reste utile en environnement type sandbox.

---

## 11. Gestion des erreurs JSON-RPC

| Cas | code | message |
|-----|------|---------|
| JSON invalide (parse) | -32700 | "Parse error" |
| Requête invalide (pas d’objet, pas de `jsonrpc`) | -32600 | "Invalid Request" |
| Méthode inconnue | -32601 | "Method not found" |
| `tools/call` avec `name` inconnu | -32602 | "Unknown tool: <name>" |
| `tools/call` avec `arguments` non objet ou invalide | -32602 | "Invalid arguments" (ou détail) |
| `initialize` avec version non supportée | -32602 | "Unsupported protocol version" |

Pour les erreurs métier (tâche introuvable, etc.) : `result` avec `isError: true` et `content[].text` = message stderr, comme décrit en 4.2.

---

## 12. Ordre de mise en œuvre suggéré

1. **`src/mcp.hpp` / `src/mcp.cpp` (squelette)**  
   - `run_mcp_server()` : boucle `getline` sur stdin, parse JSON, détection de `method`.  
   - Implémenter uniquement `initialize` et sa réponse.  
   - Renvoyer une erreur pour les autres méthodes.  
   - Tester en manuel : `echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"test","version":"1.0"}}}' | taskman mcp`

2. **`notifications/initialized`**  
   - Réception et passage en phase “opération”.

3. **`tools/list`**  
   - Table statique des 13 outils (nom, description, inputSchema).  
   - Réponse `tools/list` avec ce tableau.

4. **`tools/call` (un seul outil)**  
   - Ex. `taskman_phase_list` : argv `[]`, pas de DB si phase:list nécessite la DB — en fait phase:list lit la DB. Donc ouvrir la DB, `cmd_phase_list(0, nullptr, db)`.  
   - cxxopts : `argc=0` peut poser problème. Vérifier : `cmd_phase_list` ne parse pas d’options ; pour `argc=0, argv` on peut passer `argv` = `[nullptr]` ou un `argv` à un élément pointant sur une chaîne factice (certains parseurs s’attendent à `argv[0]`). À tester : avec `argc=0, argv=nullptr` certaines implémentations plantent. On peut passer `argc=1, argv=["taskman"]` par sécurité.  
   - Pour `phase:list`, les options sont vides ; `opts.parse(1, ["taskman"])` ou équivalent. Vérifier dans phase.cpp : pas de cxxopts pour phase:list, juste un test `--help`. Donc `argc=0` peut suffire. À valider.  
   - Capturer stdout/stderr (Option C), former `CallToolResult`.

5. **Généraliser `tools/call`**  
   - Table de correspondance `name` → (cmd_*, positionals, options).  
   - Construction `argv` à partir de `arguments` (positionals puis `--key value`).  
   - Conversion types (number → string pour `sort-order`, `reached`).  
   - Tester tous les outils un par un.

6. **`ping`**  
   - Répondre `result: {}` pour `ping`.

7. **Intégration `main`**  
   - Commande `mcp`, usage, aide.

8. **Tests**  
   - Script ou test manuel : init, phase:add, phase:list, task:add, task:list, task:get, task:edit, task:dep:add, task:dep:remove, milestone:*, etc., via `tools/call` en JSON.  
   - Vérifier `isError: true` pour une tâche inexistante, phase inexistante, etc.

9. **Documentation**  
   - README ou USAGE : section “MCP” décrivant `taskman mcp`, la config Cursor, et les variables d’environnement.

---

## 13. Récapitulatif des points de code

| Fichier / zone | Rôle |
|----------------|------|
| **`src/mcp.hpp`** | `run_mcp_server()`, déclarations des structures de config des outils si besoin. |
| **`src/mcp.cpp`** | Boucle stdio, parse JSON-RPC, `initialize`, `notifications/initialized`, `tools/list`, `tools/call`, `ping` ; table des 13 outils (nom, description, inputSchema, positionals, options) ; conversion `arguments` → `argv` ; capture cout/cerr et appel des `cmd_*` ; formation des réponses et erreurs JSON-RPC. |
| **`src/main.cpp`** | Ajout de la branche `mcp` dans le dispatch, et de `mcp` dans `COMMANDS` / usage. |
| **`CMakeLists.txt`** | Ajouter `src/mcp.cpp` dans `add_executable(taskman ...)`. |
| **`cmd_*` (phase, milestone, task, db)** | Aucune modification nécessaire si on utilise la capture cout/cerr. |

---

## 14. Évolutions possibles (hors scope initial)

- **Resources** : exposer `taskman:///task/<id>`, `taskman:///phases`, etc., pour lecture directe par le client.  
- **Prompts** : templates du type “Liste les tâches à faire pour le rôle X”.  
- **Transport Streamable HTTP** : réutiliser une base type `taskman web` avec un endpoint MCP (SSE + POST).  
- **Pagination** dans `tools/list` si la liste d’outils grossit.  
- **Paramètre `--db`** ou variable d’env dans `taskman mcp` pour override la DB (déjà couvert par `TASKMAN_DB_NAME`).
