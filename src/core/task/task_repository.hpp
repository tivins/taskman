/**
 * TaskRepository — accès à la base de données pour les tâches uniquement.
 * Responsabilité unique : opérations CRUD sur la table tasks et task_deps.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_TASK_REPOSITORY_HPP
#define TASKMAN_TASK_REPOSITORY_HPP

#include "infrastructure/db/query_executor.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class TaskRepository {
public:
    /** Constructeur prenant une référence à QueryExecutor. */
    explicit TaskRepository(QueryExecutor& executor) : executor_(executor) {}

    TaskRepository(const TaskRepository&) = delete;
    TaskRepository& operator=(const TaskRepository&) = delete;

    /** Insère une nouvelle tâche dans la base de données.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add(const std::string& id,
             const std::string& phase_id,
             const std::optional<std::string>& milestone_id,
             const std::string& title,
             const std::optional<std::string>& description,
             const std::string& status,
             std::optional<int> sort_order,
             const std::optional<std::string>& role,
             const std::optional<std::string>& creator = std::nullopt);

    /** Récupère une tâche par son ID.
     * Retourne un map vide si la tâche n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_by_id(const std::string& id);

    /** Liste les tâches avec filtres optionnels.
     * blocked_filter: "blocked" = only tasks blocked by a non-done dependency, "unblocked" = only non-blocked.
     * done_filter: "done" = only status=done, "not_done" = only status != done, "all" or empty = no filter. */
    std::vector<std::map<std::string, std::optional<std::string>>> list(
        const std::optional<std::string>& phase_id = std::nullopt,
        const std::optional<std::string>& status = std::nullopt,
        const std::optional<std::string>& role = std::nullopt,
        const std::optional<std::string>& blocked_filter = std::nullopt,
        const std::optional<std::string>& done_filter = std::nullopt);

    /** Liste les tâches avec filtres optionnels et pagination.
     * blocked_filter: "blocked" = only blocked tasks, "unblocked" = only non-blocked.
     * done_filter: "done" | "not_done" | "all" (or empty). */
    std::vector<std::map<std::string, std::optional<std::string>>> list_paginated(
        const std::optional<std::string>& phase_id = std::nullopt,
        const std::optional<std::string>& milestone_id = std::nullopt,
        const std::optional<std::string>& status = std::nullopt,
        const std::optional<std::string>& role = std::nullopt,
        const std::optional<std::string>& blocked_filter = std::nullopt,
        const std::optional<std::string>& done_filter = std::nullopt,
        int limit = 50,
        int offset = 0);

    /** Compte les tâches avec filtres optionnels.
     * blocked_filter: "blocked" | "unblocked". done_filter: "done" | "not_done" | "all". */
    int count(
        const std::optional<std::string>& phase_id = std::nullopt,
        const std::optional<std::string>& milestone_id = std::nullopt,
        const std::optional<std::string>& status = std::nullopt,
        const std::optional<std::string>& role = std::nullopt,
        const std::optional<std::string>& blocked_filter = std::nullopt,
        const std::optional<std::string>& done_filter = std::nullopt);

    /** Met à jour une tâche existante.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update(const std::string& id,
                const std::optional<std::string>& title = std::nullopt,
                const std::optional<std::string>& description = std::nullopt,
                const std::optional<std::string>& status = std::nullopt,
                const std::optional<std::string>& role = std::nullopt,
                const std::optional<std::string>& milestone_id = std::nullopt,
                const std::optional<int>& sort_order = std::nullopt,
                const std::optional<std::string>& creator = std::nullopt);

    /** Ajoute une dépendance entre deux tâches.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add_dependency(const std::string& task_id, const std::string& depends_on);

    /** Supprime une dépendance entre deux tâches.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool remove_dependency(const std::string& task_id, const std::string& depends_on);

    /** Vérifie si une tâche existe.
     * Retourne true si la tâche existe, false sinon. */
    bool exists(const std::string& id);

    /** Récupère les dépendances d'une tâche.
     * Retourne un vecteur de maps avec task_id et depends_on. */
    std::vector<std::map<std::string, std::optional<std::string>>> get_dependencies(const std::string& task_id);

    /** Récupère les UID des notes liées à une tâche (table task_notes).
     * Retourne un vecteur d'IDs de notes, ordonné par created_at puis id. */
    std::vector<std::string> get_note_ids_by_task_id(const std::string& task_id);

    /** Liste toutes les dépendances avec pagination.
     * Retourne un vecteur de maps avec task_id et depends_on. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_dependencies(
        const std::optional<std::string>& task_id = std::nullopt,
        int limit = 100,
        int offset = 0);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_TASK_REPOSITORY_HPP */
