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

bool PhaseRepository::add(const std::string& id,
                          const std::string& name,
                          const std::string& status,
                          std::optional<int> sort_order) {
    const char* sql = "INSERT INTO phases (id, name, status, sort_order) VALUES (?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(name);
    params.push_back(status);
    params.push_back(sort_order.has_value()
                     ? std::optional<std::string>(std::to_string(*sort_order))
                     : std::nullopt);
    return executor_.run(sql, params);
}

std::vector<std::map<std::string, std::optional<std::string>>> PhaseRepository::list(int limit, int offset) {
    return executor_.query(
        "SELECT id, name, status, sort_order, created_at, updated_at FROM phases ORDER BY sort_order LIMIT ? OFFSET ?",
        {std::to_string(limit), std::to_string(offset)});
}

bool PhaseRepository::update(const std::string& id,
                              const std::optional<std::string>& name,
                              const std::optional<std::string>& status,
                              const std::optional<int>& sort_order) {
    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (name.has_value()) {
        set_parts.push_back("name = ?");
        params.push_back(*name);
    }
    if (status.has_value()) {
        set_parts.push_back("status = ?");
        params.push_back(*status);
    }
    if (sort_order.has_value()) {
        set_parts.push_back("sort_order = ?");
        params.push_back(std::to_string(*sort_order));
    }

    if (set_parts.empty()) {
        return true; // Rien à mettre à jour
    }

    set_parts.push_back("updated_at = datetime('now')");

    std::string sql = "UPDATE phases SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    return executor_.run(sql.c_str(), params);
}

bool PhaseRepository::exists(const std::string& id) {
    auto rows = executor_.query("SELECT 1 FROM phases WHERE id = ?", {id});
    return !rows.empty();
}

} // namespace taskman
