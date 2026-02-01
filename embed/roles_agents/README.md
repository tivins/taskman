# Agents de rôles taskman

Ce dossier contient les définitions d'agents Cursor pour chaque rôle du système taskman. Chaque agent est configuré pour utiliser le MCP taskman avec son rôle spécifique.

## Rôles disponibles

### Gestion et coordination
- **[project-manager.md](taskman-project-manager.md)** : Gestion de projet, planification, coordination
- **[product-owner.md](taskman-product-owner.md)** : Vision produit, backlog, priorisation

### Conception et design
- **[project-designer.md](taskman-project-designer.md)** : Conception fonctionnelle, spécifications
- **[art-director.md](taskman-art-director.md)** : Direction artistique, identité visuelle
- **[ui-designer.md](taskman-ui-designer.md)** : Design d'interface, composants visuels
- **[ux-designer.md](taskman-ux-designer.md)** : Expérience utilisateur, ergonomie

### Technique
- **[software-architect.md](taskman-software-architect.md)** : Architecture technique, choix technologiques
- **[developer.md](taskman-developer.md)** : Implémentation, développement
- **[devops-engineer.md](taskman-devops-engineer.md)** : Infrastructure, CI/CD, déploiements
- **[security-engineer.md](taskman-security-engineer.md)** : Sécurité, audits, vulnérabilités
- **[qa-engineer.md](taskman-qa-engineer.md)** : Tests, qualité, validation

### Communication et documentation
- **[summary-writer.md](taskman-summary-writer.md)** : Résumés, rapports, synthèses
- **[documentation-writer.md](taskman-documentation-writer.md)** : Documentation utilisateur et technique
- **[community-manager.md](taskman-community-manager.md)** : Gestion de communauté, communication externe

## Utilisation dans Cursor

Pour utiliser un agent de rôle dans Cursor :

1. **Configurer le MCP taskman** dans votre configuration Cursor (voir [usage_mcp.md](../../docs/usage_mcp.md))

2. **Activer l'agent** en référençant le fichier correspondant dans votre contexte ou en l'ajoutant comme skill/règle

3. **L'agent utilisera automatiquement** le MCP taskman avec le rôle approprié pour :
   - Récupérer les tâches assignées au rôle
   - Créer de nouvelles tâches avec le bon rôle
   - Filtrer et consulter les données pertinentes
   - Mettre à jour l'état des tâches

## Exemple d'utilisation

Pour agir en tant que **developer** :

1. Référencez le fichier `taskman-developer.md` dans votre contexte Cursor
2. L'agent utilisera automatiquement `taskman_task_list` avec `role: "developer"` pour récupérer vos tâches
3. Vous pouvez demander à l'agent de :
   - "Montre-moi mes tâches à faire"
   - "Marque la tâche X comme en cours"
   - "Quelles sont mes tâches terminées cette semaine ?"

## Structure des fichiers

Chaque fichier d'agent décrit le **rôle** (nom, responsabilités, comportement, relations) et renvoie aux règles partagées pour la mécanique MCP :

- **Rôle et responsabilités** : Description du rôle dans le projet
- **Utilisation du MCP taskman** : Référence aux règles **embed/rules/taskman-mcp-usage.mdc** (lister, consulter, modifier les tâches) et **embed/rules/task-creation.mdc** (création de tâches avec creator/role)
- **Comportement attendu** : Comment l'agent doit agir
- **Relations avec les autres rôles** : Comment collaborer
- **Outils MCP disponibles** : Liste des outils pertinents pour le rôle

Les détails des appels MCP (taskman_task_list, taskman_task_add, etc.) et les règles de création de tâches (CLI et MCP) sont centralisés dans **embed/rules/** pour éviter la duplication et garder une seule source de vérité.

## Règles d'assignation des tâches

Voir **embed/rules/task-creation.mdc** pour la règle complète (CLI et MCP). Résumé :

- **Project Manager** : Peut assigner des tâches à **tous les rôles** (y compris lui-même)
- **Autres rôles** : Peuvent créer des tâches soit assignées à leur propre rôle, soit non assignées (sans paramètre `role`) pour validation par le project manager
- **Validation** : Le project manager doit régulièrement consulter les tâches non assignées, les valider, les modifier si nécessaire, puis les assigner au rôle approprié
- **Creator** : Toujours renseigner `creator` avec le nom du rôle pour la traçabilité

## Notes importantes

- Chaque agent doit **toujours** utiliser le MCP taskman avec son rôle spécifique pour filtrer ses propres tâches
- Pour l'usage des outils MCP (list, get, edit, statuts), voir **embed/rules/taskman-mcp-usage.mdc**
- Pour la création de tâches (creator, role, tâche non assignée), voir **embed/rules/task-creation.mdc**
- Tous les agents ont accès aux mêmes outils MCP, mais avec des perspectives et permissions différentes
