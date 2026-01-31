/**
 * TaskService — logique métier pour les tâches uniquement.
 * Responsabilité unique : règles métier, validation, génération d'identifiants.
 * Utilise TaskRepository pour l'accès aux données.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_TASK_SERVICE_HPP
#define TASKMAN_TASK_SERVICE_HPP

#include "task_repository.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class TaskService {
public:
    /** Constructeur prenant une référence à TaskRepository. */
    explicit TaskService(TaskRepository& repository) : repository_(repository) {}

    TaskService(const TaskService&) = delete;
    TaskService& operator=(const TaskService&) = delete;

    /** Crée une nouvelle tâche avec génération automatique d'ID UUID v4.
     * Effectue la validation des données avant insertion.
     * Retourne l'ID de la tâche créée, ou nullopt en cas d'erreur. */
    std::optional<std::string> create_task(
        const std::string& phase_id,
        const std::optional<std::string>& milestone_id,
        const std::string& title,
        const std::optional<std::string>& description = std::nullopt,
        const std::string& status = "to_do",
        std::optional<int> sort_order = std::nullopt,
        const std::optional<std::string>& role = std::nullopt,
        const std::optional<std::string>& creator = std::nullopt);

    /** Ajoute une tâche avec un ID spécifique (pour compatibilité avec les tests et code existant).
     * Effectue la validation des données avant insertion.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add_task_with_id(
        const std::string& id,
        const std::string& phase_id,
        const std::optional<std::string>& milestone_id,
        const std::string& title,
        const std::optional<std::string>& description = std::nullopt,
        const std::string& status = "to_do",
        std::optional<int> sort_order = std::nullopt,
        const std::optional<std::string>& role = std::nullopt,
        const std::optional<std::string>& creator = std::nullopt);

    /** Récupère une tâche par son ID.
     * Retourne un map vide si la tâche n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_task(const std::string& id);

    /** Liste les tâches avec filtres optionnels.
     * Retourne un vecteur de maps représentant les tâches. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_tasks(
        const std::optional<std::string>& phase_id = std::nullopt,
        const std::optional<std::string>& status = std::nullopt,
        const std::optional<std::string>& role = std::nullopt);

    /** Met à jour une tâche existante.
     * Effectue la validation des données avant mise à jour.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update_task(const std::string& id,
                     const std::optional<std::string>& title = std::nullopt,
                     const std::optional<std::string>& description = std::nullopt,
                     const std::optional<std::string>& status = std::nullopt,
                     const std::optional<std::string>& role = std::nullopt,
                     const std::optional<std::string>& milestone_id = std::nullopt,
                     const std::optional<int>& sort_order = std::nullopt,
                     const std::optional<std::string>& creator = std::nullopt);

    /** Ajoute une dépendance entre deux tâches.
     * Effectue la validation (tâches existantes, pas de dépendance circulaire).
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add_task_dependency(const std::string& task_id, const std::string& depends_on);

    /** Supprime une dépendance entre deux tâches.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool remove_task_dependency(const std::string& task_id, const std::string& depends_on);

    /** Valide un statut de tâche.
     * Retourne true si le statut est valide, false sinon. */
    static bool is_valid_status(const std::string& status);

    /** Génère un UUID v4.
     * Retourne une chaîne représentant l'UUID. */
    static std::string generate_uuid_v4();

private:
    TaskRepository& repository_;
};

} // namespace taskman

#endif /* TASKMAN_TASK_SERVICE_HPP */
