/**
 * MilestoneService — logique métier pour les milestones uniquement.
 * Responsabilité unique : règles métier, validation.
 * Utilise MilestoneRepository pour l'accès aux données.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_MILESTONE_SERVICE_HPP
#define TASKMAN_MILESTONE_SERVICE_HPP

#include "milestone_repository.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class MilestoneService {
public:
    /** Constructeur prenant une référence à MilestoneRepository. */
    explicit MilestoneService(MilestoneRepository& repository) : repository_(repository) {}

    MilestoneService(const MilestoneService&) = delete;
    MilestoneService& operator=(const MilestoneService&) = delete;

    /** Crée un nouveau milestone avec validation.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool create_milestone(const std::string& id,
                           const std::string& phase_id,
                           const std::optional<std::string>& name = std::nullopt,
                           const std::optional<std::string>& criterion = std::nullopt,
                           bool reached = false);

    /** Récupère un milestone par son ID.
     * Retourne un map vide si le milestone n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_milestone(const std::string& id);

    /** Liste les milestones avec filtre optionnel par phase.
     * Retourne un vecteur de maps représentant les milestones. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_milestones(
        const std::optional<std::string>& phase_id = std::nullopt);

    /** Met à jour un milestone existant.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update_milestone(const std::string& id,
                          const std::optional<std::string>& name = std::nullopt,
                          const std::optional<std::string>& criterion = std::nullopt,
                          const std::optional<bool>& reached = std::nullopt,
                          const std::optional<std::string>& phase_id = std::nullopt);

private:
    MilestoneRepository& repository_;
};

} // namespace taskman

#endif /* TASKMAN_MILESTONE_SERVICE_HPP */
