# Agents de rôles taskman

Ce dossier contient les définitions d'agents Cursor pour chaque rôle du système taskman. Chaque agent est configuré pour utiliser le MCP taskman avec son rôle spécifique.

## Rôles disponibles

### Gestion et coordination
- **[project-manager.md](project-manager.md)** : Gestion de projet, planification, coordination
- **[product-owner.md](product-owner.md)** : Vision produit, backlog, priorisation

### Conception et design
- **[project-designer.md](project-designer.md)** : Conception fonctionnelle, spécifications
- **[art-director.md](art-director.md)** : Direction artistique, identité visuelle
- **[ui-designer.md](ui-designer.md)** : Design d'interface, composants visuels
- **[ux-designer.md](ux-designer.md)** : Expérience utilisateur, ergonomie

### Technique
- **[software-architect.md](software-architect.md)** : Architecture technique, choix technologiques
- **[developer.md](developer.md)** : Implémentation, développement
- **[devops-engineer.md](devops-engineer.md)** : Infrastructure, CI/CD, déploiements
- **[security-engineer.md](security-engineer.md)** : Sécurité, audits, vulnérabilités
- **[qa-engineer.md](qa-engineer.md)** : Tests, qualité, validation

### Communication et documentation
- **[summary-writer.md](summary-writer.md)** : Résumés, rapports, synthèses
- **[documentation-writer.md](documentation-writer.md)** : Documentation utilisateur et technique
- **[community-manager.md](community-manager.md)** : Gestion de communauté, communication externe

## Utilisation dans Cursor

Pour utiliser un agent de rôle dans Cursor :

1. **Configurer le MCP taskman** dans votre configuration Cursor (voir [USAGE_MCP.md](../../USAGE_MCP.md))

2. **Activer l'agent** en référençant le fichier correspondant dans votre contexte ou en l'ajoutant comme skill/règle

3. **L'agent utilisera automatiquement** le MCP taskman avec le rôle approprié pour :
   - Récupérer les tâches assignées au rôle
   - Créer de nouvelles tâches avec le bon rôle
   - Filtrer et consulter les données pertinentes
   - Mettre à jour l'état des tâches

## Exemple d'utilisation

Pour agir en tant que **developer** :

1. Référencez le fichier `developer.md` dans votre contexte Cursor
2. L'agent utilisera automatiquement `taskman_task_list` avec `role: "developer"` pour récupérer vos tâches
3. Vous pouvez demander à l'agent de :
   - "Montre-moi mes tâches à faire"
   - "Marque la tâche X comme en cours"
   - "Quelles sont mes tâches terminées cette semaine ?"

## Structure des fichiers

Chaque fichier d'agent contient :

- **Rôle et responsabilités** : Description du rôle dans le projet
- **Utilisation du MCP taskman** : Instructions pour utiliser les outils MCP
- **Actions courantes** : Exemples d'utilisation des outils avec le rôle
- **Comportement attendu** : Comment l'agent doit agir
- **Relations avec les autres rôles** : Comment collaborer
- **Outils MCP disponibles** : Liste des outils pertinents

## Règles d'assignation des tâches

- **Project Manager** : Peut assigner des tâches à **tous les rôles** (y compris lui-même)
- **Autres rôles** : Peuvent créer des tâches soit :
  - Assignées à leur propre rôle
  - Non assignées (sans paramètre `role`) pour validation par le project manager
- **Validation** : Le project manager doit régulièrement consulter les tâches non assignées, les valider, les modifier si nécessaire, puis les assigner au rôle approprié

## Notes importantes

- Chaque agent doit **toujours** utiliser le MCP taskman avec son rôle spécifique pour filtrer ses propres tâches
- Les filtres par rôle permettent de récupérer uniquement les tâches pertinentes
- Pour créer une tâche non assignée, omettre le paramètre `role` dans l'appel MCP
- Tous les agents ont accès aux mêmes outils MCP, mais avec des perspectives et permissions différentes
