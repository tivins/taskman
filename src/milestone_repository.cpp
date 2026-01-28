/**
 * Implémentation de MilestoneRepository.
 */

#include "milestone_repository.hpp"

namespace taskman {

std::map<std::string, std::optional<std::string>> MilestoneRepository::get_by_id(const std::string& id) {
    auto rows = executor_.query(
        "SELECT id, phase_id, name, criterion, reached, created_at, updated_at FROM milestones WHERE id = ?",
        {id});
    if (rows.empty()) {
        return {};
    }
    return rows[0];
}

std::vector<std::map<std::string, std::optional<std::string>>> MilestoneRepository::list(int limit, int offset) {
    return executor_.query(
        "SELECT id, phase_id, name, criterion, reached, created_at, updated_at FROM milestones ORDER BY phase_id, id LIMIT ? OFFSET ?",
        {std::to_string(limit), std::to_string(offset)});
}

bool MilestoneRepository::exists(const std::string& id) {
    auto rows = executor_.query("SELECT 1 FROM milestones WHERE id = ?", {id});
    return !rows.empty();
}

} // namespace taskman
