# Refactorisation : Utilisation de fonctions au lieu de SQL direct

## Problème identifié

Dans `src/demo.cpp`, la fonction `cmd_demo_generate()` utilise des requêtes SQL directes pour créer des phases, milestones, tasks et dépendances, alors que des fonctions CLI existent déjà pour ces opérations :

- `cmd_phase_add()` dans `src/phase.cpp`
- `cmd_milestone_add()` dans `src/milestone.cpp`
- `cmd_task_add()` dans `src/task.cpp`
- `cmd_task_dep_add()` dans `src/task.cpp`

### Exemple actuel dans `demo.cpp`

```cpp
// Phases
if (!db.run("INSERT INTO phases (id, name, status, sort_order) VALUES (?, ?, ?, ?)",
            {"P1", "Design", "in_progress", "1"})) return 1;
if (!db.run("INSERT INTO phases (id, name, status, sort_order) VALUES (?, ?, ?, ?)",
            {"P2", "Development", "to_do", "2"})) return 1;
// ...
```

### Pourquoi les fonctions CLI ne sont pas utilisées ?

Les fonctions CLI (`cmd_phase_add`, `cmd_milestone_add`, etc.) sont conçues pour être appelées depuis la ligne de commande :

1. **Signature incompatible** : Elles prennent `int argc, char* argv[]` et utilisent `cxxopts` pour parser les arguments
2. **Usage programmatique difficile** : Il faudrait construire artificiellement des tableaux `argv` pour chaque appel
3. **Pas de couche d'abstraction** : Même les fonctions CLI utilisent directement `db.run()` avec des requêtes SQL (voir lignes 95-102 dans `phase.cpp`, 84-92 dans `milestone.cpp`)

### Problèmes de l'approche actuelle

1. **Duplication de code** : La logique d'insertion SQL est dupliquée entre les commandes CLI et `demo.cpp`
2. **Maintenance difficile** : Si le schéma de base de données change, il faut modifier plusieurs endroits
3. **Pas de validation centralisée** : Les validations (status, reached, etc.) sont dupliquées
4. **Incohérence potentielle** : Risque d'oublier certaines validations dans `demo.cpp`

## Solution proposée

Créer des fonctions utilitaires réutilisables qui :

1. **Prennent directement les paramètres** (pas `argc/argv`)
2. **Effectuent les validations nécessaires** (status, reached, roles, etc.)
3. **Sont utilisables à la fois par les commandes CLI et par `demo.cpp`**

### Architecture proposée

#### 1. Fonctions utilitaires dans les headers

Ajouter des fonctions dans `phase.hpp`, `milestone.hpp`, `task.hpp` :

```cpp
// phase.hpp
namespace taskman {
    class Database;
    
    // Fonction utilitaire (à utiliser par CLI et programmatiquement)
    bool phase_add(Database& db, 
                   const std::string& id,
                   const std::string& name,
                   const std::string& status = "to_do",
                   std::optional<int> sort_order = std::nullopt);
    
    // Commande CLI (utilise la fonction utilitaire)
    int cmd_phase_add(int argc, char* argv[], Database& db);
}
```

#### 2. Implémentation

Les fonctions CLI appelleraient les fonctions utilitaires :

```cpp
// phase.cpp
bool phase_add(Database& db, 
               const std::string& id,
               const std::string& name,
               const std::string& status,
               std::optional<int> sort_order) {
    // Validation
    if (!is_valid_status(status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    
    // Insertion SQL
    const char* sql = "INSERT INTO phases (id, name, status, sort_order) VALUES (?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(name);
    params.push_back(status);
    params.push_back(sort_order.has_value() 
                     ? std::optional<std::string>(std::to_string(*sort_order))
                     : std::nullopt);
    
    return db.run(sql, params);
}

int cmd_phase_add(int argc, char* argv[], Database& db) {
    // ... parsing avec cxxopts ...
    
    // Appel de la fonction utilitaire
    if (!phase_add(db, id, name, status, 
                   sort_order_str.empty() ? std::nullopt 
                                          : std::optional<int>(std::stoi(sort_order_str)))) {
        return 1;
    }
    return 0;
}
```

#### 3. Utilisation dans `demo.cpp`

```cpp
// demo.cpp
#include "phase.hpp"
#include "milestone.hpp"
#include "task.hpp"

int cmd_demo_generate(int argc, char* argv[], Database& db) {
    // ...
    
    // Phases
    if (!phase_add(db, "P1", "Design", "in_progress", 1)) return 1;
    if (!phase_add(db, "P2", "Development", "to_do", 2)) return 1;
    if (!phase_add(db, "P3", "Acceptance", "to_do", 3)) return 1;
    if (!phase_add(db, "P4", "Delivery", "to_do", 4)) return 1;
    
    // Milestones
    if (!milestone_add(db, "M1", "P1", "Specs approved", 
                       "Document signed off by client", true)) return 1;
    // ...
    
    // Tasks
    std::string t1 = generate_uuid_v4();
    if (!task_add(db, t1, "P1", "M1", "Write requirements document",
                  std::nullopt, "done", 1, "project-manager")) return 1;
    // ...
}
```

## Avantages de cette approche

1. **Réutilisabilité** : Les fonctions peuvent être utilisées depuis n'importe où dans le code
2. **Maintenance simplifiée** : Un seul endroit pour la logique d'insertion et les validations
3. **Cohérence** : Garantit que les mêmes validations sont appliquées partout
4. **Testabilité** : Les fonctions utilitaires peuvent être testées indépendamment
5. **Lisibilité** : Le code dans `demo.cpp` devient plus expressif et moins verbeux

## Fonctions à créer

### Phases
- `bool phase_add(Database& db, const std::string& id, const std::string& name, 
                  const std::string& status = "to_do", std::optional<int> sort_order = std::nullopt)`

### Milestones
- `bool milestone_add(Database& db, const std::string& id, const std::string& phase_id,
                      const std::optional<std::string>& name = std::nullopt,
                      const std::optional<std::string>& criterion = std::nullopt,
                      bool reached = false)`

### Tasks
- `bool task_add(Database& db, const std::string& id, const std::string& phase_id,
                 const std::optional<std::string>& milestone_id,
                 const std::string& title,
                 const std::optional<std::string>& description = std::nullopt,
                 const std::string& status = "to_do",
                 std::optional<int> sort_order = std::nullopt,
                 const std::optional<std::string>& role = std::nullopt)`

### Task Dependencies
- `bool task_dep_add(Database& db, const std::string& task_id, const std::string& depends_on)`

## Notes d'implémentation

1. **Gestion d'erreurs** : Les fonctions utilitaires peuvent soit retourner `bool` (comme proposé), soit lever des exceptions. Le choix dépend de la stratégie d'erreur du projet.

2. **Messages d'erreur** : Les fonctions utilitaires peuvent écrire sur `stderr` directement, ou retourner des codes d'erreur plus détaillés.

3. **Validation** : Toutes les validations existantes dans les fonctions CLI doivent être présentes dans les fonctions utilitaires :
   - Status : `to_do`, `in_progress`, `done`
   - Reached : `0` ou `1`
   - Roles : validation via `is_valid_role()`
   - Sort order : entier valide

4. **Rétrocompatibilité** : Les fonctions CLI existantes continueront de fonctionner, elles appelleront simplement les nouvelles fonctions utilitaires.

## État actuel

- [ ] Fonctions utilitaires créées dans les headers
- [ ] Implémentation des fonctions utilitaires
- [ ] Refactorisation des fonctions CLI pour utiliser les utilitaires
- [ ] Refactorisation de `demo.cpp` pour utiliser les utilitaires
- [ ] Tests unitaires pour les fonctions utilitaires
- [ ] Documentation mise à jour

## Références

- `src/demo.cpp` : Utilise actuellement SQL direct
- `src/phase.cpp` : Contient `cmd_phase_add()` avec logique SQL
- `src/milestone.cpp` : Contient `cmd_milestone_add()` avec logique SQL
- `src/task.cpp` : Contient `cmd_task_add()` et `cmd_task_dep_add()` avec logique SQL
