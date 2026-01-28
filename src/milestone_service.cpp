/**
 * Implémentation de MilestoneService.
 */

#include "milestone_service.hpp"

namespace taskman {

bool MilestoneService::create_milestone(const std::string& id,
                                        const std::string& phase_id,
                                        const std::optional<std::string>& name,
                                        const std::optional<std::string>& criterion,
                                        bool reached) {
    return repository_.add(id, phase_id, name, criterion, reached);
}

std::map<std::string, std::optional<std::string>> MilestoneService::get_milestone(const std::string& id) {
    return repository_.get_by_id(id);
}

std::vector<std::map<std::string, std::optional<std::string>>> MilestoneService::list_milestones(
    const std::optional<std::string>& phase_id) {
    if (phase_id.has_value()) {
        return repository_.list_by_phase(*phase_id);
    }
    // Utilise une limite élevée pour obtenir tous les milestones
    return repository_.list(10000, 0);
}

bool MilestoneService::update_milestone(const std::string& id,
                                        const std::optional<std::string>& name,
                                        const std::optional<std::string>& criterion,
                                        const std::optional<bool>& reached,
                                        const std::optional<std::string>& phase_id) {
    return repository_.update(id, name, criterion, reached, phase_id);
}

} // namespace taskman
