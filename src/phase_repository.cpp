/**
 * Implémentation de PhaseRepository.
 */

#include "phase_repository.hpp"

namespace taskman {

std::map<std::string, std::optional<std::string>> PhaseRepository::get_by_id(const std::string& id) {
    auto rows = executor_.query(
        "SELECT id, name, status, sort_order, created_at, updated_at FROM phases WHERE id = ?",
        {id});
    if (rows.empty()) {
        return {};
    }
    return rows[0];
}

std::vector<std::map<std::string, std::optional<std::string>>> PhaseRepository::list(int limit, int offset) {
    return executor_.query(
        "SELECT id, name, status, sort_order, created_at, updated_at FROM phases ORDER BY sort_order LIMIT ? OFFSET ?",
        {std::to_string(limit), std::to_string(offset)});
}

bool PhaseRepository::exists(const std::string& id) {
    auto rows = executor_.query("SELECT 1 FROM phases WHERE id = ?", {id});
    return !rows.empty();
}

} // namespace taskman
