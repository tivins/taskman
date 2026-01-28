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

bool MilestoneRepository::add(const std::string& id,
                               const std::string& phase_id,
                               const std::optional<std::string>& name,
                               const std::optional<std::string>& criterion,
                               bool reached) {
    const char* sql = "INSERT INTO milestones (id, phase_id, name, criterion, reached) VALUES (?, ?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(phase_id);
    params.push_back(name);
    params.push_back(criterion);
    params.push_back(reached ? "1" : "0");
    return executor_.run(sql, params);
}

std::vector<std::map<std::string, std::optional<std::string>>> MilestoneRepository::list(int limit, int offset) {
    return executor_.query(
        "SELECT id, phase_id, name, criterion, reached, created_at, updated_at FROM milestones ORDER BY phase_id, id LIMIT ? OFFSET ?",
        {std::to_string(limit), std::to_string(offset)});
}

std::vector<std::map<std::string, std::optional<std::string>>> MilestoneRepository::list_by_phase(const std::string& phase_id) {
    return executor_.query(
        "SELECT id, phase_id, name, criterion, reached, created_at, updated_at FROM milestones WHERE phase_id = ? ORDER BY phase_id, id",
        {phase_id});
}

bool MilestoneRepository::update(const std::string& id,
                                  const std::optional<std::string>& name,
                                  const std::optional<std::string>& criterion,
                                  const std::optional<bool>& reached,
                                  const std::optional<std::string>& phase_id) {
    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (name.has_value()) {
        set_parts.push_back("name = ?");
        params.push_back(*name);
    }
    if (criterion.has_value()) {
        set_parts.push_back("criterion = ?");
        params.push_back(*criterion);
    }
    if (reached.has_value()) {
        set_parts.push_back("reached = ?");
        params.push_back(*reached ? "1" : "0");
    }
    if (phase_id.has_value()) {
        set_parts.push_back("phase_id = ?");
        params.push_back(*phase_id);
    }

    if (set_parts.empty()) {
        return true; // Rien à mettre à jour
    }

    set_parts.push_back("updated_at = datetime('now')");

    std::string sql = "UPDATE milestones SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    return executor_.run(sql.c_str(), params);
}

bool MilestoneRepository::exists(const std::string& id) {
    auto rows = executor_.query("SELECT 1 FROM milestones WHERE id = ?", {id});
    return !rows.empty();
}

} // namespace taskman
