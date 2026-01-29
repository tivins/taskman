# ADR-0001: Report de l'implémentation d'interfaces abstraites et d'injection de dépendances

**Date**: 2026-01-28  
**Statut**: Accepté  
**Décideurs**: [@Tivins](https://github.com/tivins), Équipe de développement  
**Tags**: architecture, SOLID, YAGNI, dépendances

## Contexte

Lors de l'analyse SOLID du code (voir `docs/internal/analysis-2026-01-28-solid.md`), plusieurs violations du principe de Dependency Inversion (DIP) ont été identifiées :

1. **Dépendances directes aux implémentations concrètes** :
   - `Database` est couplé directement à SQLite
   - Les repositories (`TaskRepository`, `PhaseRepository`, etc.) sont des classes concrètes sans interfaces abstraites
   - `WebServer` dépend directement de `httplib::Server`
   - Les command parsers dépendent directement de `cxxopts::Options`

2. **Instanciation directe dans le code** :
   - `main.cpp` instancie directement `Database`
   - `web.cpp` instancie directement `WebServer` et les contrôleurs
   - `mcp.cpp` instancie directement les handlers

3. **Impact sur la testabilité** :
   - Impossible d'injecter des mocks pour les tests unitaires
   - Tests limités aux tests d'intégration avec la vraie base de données

## Décision

**Nous décidons de reporter l'implémentation d'interfaces abstraites et d'injection de dépendances pour l'instant.**

### Décisions spécifiques

1. **Pas d'interfaces abstraites pour Database** : Ne pas créer `IDatabase` tant qu'il n'y a pas de besoin réel de supporter d'autres SGBD que SQLite.

2. **Pas d'interfaces abstraites pour les Repositories** : Ne pas créer `ITaskRepository`, `IPhaseRepository`, etc. Les repositories concrets sont suffisants pour les besoins actuels.

3. **Pas d'interfaces abstraites pour les dépendances externes** : Ne pas créer d'interfaces pour `httplib::Server` ou `cxxopts::Options` tant qu'il n'y a pas de besoin réel de changer de bibliothèque.

4. **Pas d'injection de dépendances** : Ne pas implémenter l'injection de dépendances dans `main.cpp`, `web.cpp`, `mcp.cpp` pour l'instant.

## Justification

### Principe YAGNI (You Aren't Gonna Need It)

> "The best code is no code at all. But if you must write code, write the simplest code that works. You aren't gonna need those fancy abstractions until you actually need them."  
> — Adaptation libre de Jeff Atwood et Ron Jeffries


- **Pas de besoin réel immédiat** : Aucun besoin actuel de supporter plusieurs SGBD, de changer de bibliothèque HTTP, ou d'utiliser des mocks dans les tests.

- **Tests fonctionnels** : Les tests existants utilisent déjà SQLite en mémoire (`:memory:`) et fonctionnent correctement. Les tests d'intégration actuels sont suffisants pour valider le comportement du système.

- **Complexité vs bénéfice** : L'implémentation d'interfaces abstraites et d'injection de dépendances ajouterait :
  - ~15-20 nouveaux fichiers (interfaces + implémentations)
  - Complexité supplémentaire dans la construction et l'initialisation
  - Maintenance supplémentaire
  - Sans bénéfice réel immédiat

- **Cohérence** : Cette décision est cohérente avec la décision précédente de ne pas implémenter le Strategy pattern pour les formatters (ADR implicite dans l'analyse SOLID).

### État actuel du code

Le code respecte déjà plusieurs principes SOLID :
- **SRP** : Responsabilités bien séparées (repositories, services, formatters, parsers)
- **OCP** : Extension possible via le pattern Command pour les nouvelles commandes
- **DIP** : Dépendances directes aux implémentations, mais acceptables pour l'instant

## Conséquences

### Positives

- **Simplicité** : Code plus simple à comprendre et maintenir
- **Moins de fichiers** : Moins de complexité dans la structure du projet
- **Développement plus rapide** : Pas de temps passé sur des abstractions non nécessaires
- **Tests fonctionnels** : Les tests d'intégration avec SQLite en mémoire fonctionnent correctement

### Négatives

- **Couplage fort** : Le code reste couplé à SQLite, httplib, et cxxopts
- **Testabilité limitée** : Pas de possibilité d'utiliser des mocks pour des tests unitaires isolés
- **Flexibilité réduite** : Changement de bibliothèque nécessiterait une refactorisation importante
- **Violation partielle du DIP** : Le principe de Dependency Inversion n'est pas complètement respecté

### Mitigations

- **Tests d'intégration** : Les tests utilisent SQLite en mémoire, ce qui permet des tests rapides et isolés
- **Architecture modulaire** : La séparation en repositories, services, formatters facilite les futures refactorisations si nécessaire
- **Révision future** : Cette décision sera révisée si un besoin réel apparaît (ex: besoin de supporter PostgreSQL, besoin de mocks pour tests unitaires)

## Alternatives considérées

### Alternative 1 : Implémenter toutes les interfaces abstraites maintenant

**Avantages** :
- Respect complet du DIP
- Testabilité maximale avec mocks
- Flexibilité maximale pour changer d'implémentation

**Inconvénients** :
- Complexité importante (~20 nouveaux fichiers)
- Pas de besoin réel immédiat
- Maintenance supplémentaire
- Violation du principe YAGNI

**Décision** : Rejeté - trop de complexité pour un bénéfice hypothétique

### Alternative 2 : Implémenter uniquement les interfaces critiques

**Avantages** :
- Compromis entre simplicité et flexibilité
- Focus sur les dépendances les plus critiques

**Inconvénients** :
- Quelles interfaces sont vraiment critiques ?
- Risque de créer des interfaces qui ne seront jamais utilisées
- Incohérence dans l'architecture

**Décision** : Rejeté - difficile de déterminer quelles interfaces sont vraiment nécessaires sans besoin concret

## Références

- Analyse SOLID complète : `docs/internal/analysis-2026-01-28-solid.md`
- Principe YAGNI : [Wikipedia - You Aren't Gonna Need It](https://en.wikipedia.org/wiki/You_aren%27t_gonna_need_it)
- Principe SOLID - Dependency Inversion : [Wikipedia - Dependency Inversion Principle](https://en.wikipedia.org/wiki/Dependency_inversion_principle)

## Notes

Cette décision peut être révisée si :
- Un besoin réel apparaît de supporter plusieurs SGBD
- Un besoin réel apparaît de changer de bibliothèque HTTP ou de parsing CLI
- Un besoin réel apparaît d'utiliser des mocks pour des tests unitaires isolés
- L'équipe décide d'investir dans une meilleure testabilité

Dans ce cas, une nouvelle ADR sera créée pour documenter le changement de décision.
