/**
 * MilestoneRepository — accès à la base de données pour les milestones uniquement.
 * Responsabilité unique : opérations CRUD sur la table milestones.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_MILESTONE_REPOSITORY_HPP
#define TASKMAN_MILESTONE_REPOSITORY_HPP

#include "query_executor.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class MilestoneRepository {
public:
    /** Constructeur prenant une référence à QueryExecutor. */
    explicit MilestoneRepository(QueryExecutor& executor) : executor_(executor) {}

    MilestoneRepository(const MilestoneRepository&) = delete;
    MilestoneRepository& operator=(const MilestoneRepository&) = delete;

    /** Récupère un milestone par son ID.
     * Retourne un map vide si le milestone n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_by_id(const std::string& id);

    /** Insère un nouveau milestone dans la base de données.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add(const std::string& id,
             const std::string& phase_id,
             const std::optional<std::string>& name,
             const std::optional<std::string>& criterion,
             bool reached);

    /** Liste les milestones avec pagination.
     * Retourne un vecteur de maps représentant les milestones. */
    std::vector<std::map<std::string, std::optional<std::string>>> list(int limit = 30, int offset = 0);

    /** Liste les milestones filtrés par phase_id.
     * Retourne un vecteur de maps représentant les milestones. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_by_phase(const std::string& phase_id);

    /** Met à jour un milestone existant.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update(const std::string& id,
                const std::optional<std::string>& name = std::nullopt,
                const std::optional<std::string>& criterion = std::nullopt,
                const std::optional<bool>& reached = std::nullopt,
                const std::optional<std::string>& phase_id = std::nullopt);

    /** Vérifie si un milestone existe.
     * Retourne true si le milestone existe, false sinon. */
    bool exists(const std::string& id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_MILESTONE_REPOSITORY_HPP */
