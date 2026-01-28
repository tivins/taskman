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

    /** Liste les milestones avec pagination.
     * Retourne un vecteur de maps représentant les milestones. */
    std::vector<std::map<std::string, std::optional<std::string>>> list(int limit = 30, int offset = 0);

    /** Vérifie si un milestone existe.
     * Retourne true si le milestone existe, false sinon. */
    bool exists(const std::string& id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_MILESTONE_REPOSITORY_HPP */
