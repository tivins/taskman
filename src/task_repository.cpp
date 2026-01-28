/**
 * Implémentation de TaskRepository.
 */

#include "task_repository.hpp"
#include <iostream>

namespace taskman {

bool TaskRepository::add(const std::string& id,
                         const std::string& phase_id,
                         const std::optional<std::string>& milestone_id,
                         const std::string& title,
                         const std::optional<std::string>& description,
                         const std::string& status,
                         std::optional<int> sort_order,
                         const std::optional<std::string>& role) {
    const char* sql = "INSERT INTO tasks (id, phase_id, milestone_id, title, description, status, sort_order, role) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(phase_id);
    params.push_back(milestone_id);
    params.push_back(title);
    params.push_back(description);
    params.push_back(status);
    params.push_back(sort_order.has_value()
                     ? std::optional<std::string>(std::to_string(*sort_order))
                     : std::nullopt);
    params.push_back(role);
    return executor_.run(sql, params);
}

std::map<std::string, std::optional<std::string>> TaskRepository::get_by_id(const std::string& id) {
    auto rows = executor_.query(
        "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role, created_at, updated_at FROM tasks WHERE id = ?",
        {id});
    if (rows.empty()) {
        return {};
    }
    return rows[0];
}

std::vector<std::map<std::string, std::optional<std::string>>> TaskRepository::list(
    const std::optional<std::string>& phase_id,
    const std::optional<std::string>& status,
    const std::optional<std::string>& role) {
    std::string sql = "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role, created_at, updated_at FROM tasks";
    std::vector<std::string> where_parts;
    std::vector<std::optional<std::string>> params;

    if (phase_id.has_value()) {
        where_parts.push_back("phase_id = ?");
        params.push_back(*phase_id);
    }
    if (status.has_value()) {
        where_parts.push_back("status = ?");
        params.push_back(*status);
    }
    if (role.has_value()) {
        where_parts.push_back("role = ?");
        params.push_back(*role);
    }
    if (!where_parts.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_parts.size(); ++i) {
            if (i) sql += " AND ";
            sql += where_parts[i];
        }
    }
    sql += " ORDER BY phase_id, milestone_id, sort_order, id";

    if (params.empty()) {
        return executor_.query(sql.c_str());
    } else {
        return executor_.query(sql.c_str(), params);
    }
}

std::vector<std::map<std::string, std::optional<std::string>>> TaskRepository::list_paginated(
    const std::optional<std::string>& phase_id,
    const std::optional<std::string>& milestone_id,
    const std::optional<std::string>& status,
    const std::optional<std::string>& role,
    int limit,
    int offset) {
    std::string sql = "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role, created_at, updated_at FROM tasks";
    std::vector<std::string> where_parts;
    std::vector<std::optional<std::string>> params;

    if (phase_id.has_value()) {
        where_parts.push_back("phase_id = ?");
        params.push_back(*phase_id);
    }
    if (milestone_id.has_value()) {
        where_parts.push_back("milestone_id = ?");
        params.push_back(*milestone_id);
    }
    if (status.has_value()) {
        where_parts.push_back("status = ?");
        params.push_back(*status);
    }
    if (role.has_value()) {
        where_parts.push_back("role = ?");
        params.push_back(*role);
    }
    if (!where_parts.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_parts.size(); ++i) {
            if (i) sql += " AND ";
            sql += where_parts[i];
        }
    }
    sql += " ORDER BY phase_id, milestone_id, sort_order, id LIMIT ? OFFSET ?";
    params.push_back(std::to_string(limit));
    params.push_back(std::to_string(offset));

    return executor_.query(sql.c_str(), params);
}

int TaskRepository::count(
    const std::optional<std::string>& phase_id,
    const std::optional<std::string>& milestone_id,
    const std::optional<std::string>& status,
    const std::optional<std::string>& role) {
    std::string sql = "SELECT COUNT(*) as count FROM tasks";
    std::vector<std::string> where_parts;
    std::vector<std::optional<std::string>> params;

    if (phase_id.has_value()) {
        where_parts.push_back("phase_id = ?");
        params.push_back(*phase_id);
    }
    if (milestone_id.has_value()) {
        where_parts.push_back("milestone_id = ?");
        params.push_back(*milestone_id);
    }
    if (status.has_value()) {
        where_parts.push_back("status = ?");
        params.push_back(*status);
    }
    if (role.has_value()) {
        where_parts.push_back("role = ?");
        params.push_back(*role);
    }
    if (!where_parts.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_parts.size(); ++i) {
            if (i) sql += " AND ";
            sql += where_parts[i];
        }
    }

    auto rows = executor_.query(sql.c_str(), params);
    if (rows.empty() || !rows[0].count("count")) {
        return 0;
    }
    try {
        return std::stoi(rows[0].at("count").value_or("0"));
    } catch (...) {
        return 0;
    }
}

bool TaskRepository::update(const std::string& id,
                            const std::optional<std::string>& title,
                            const std::optional<std::string>& description,
                            const std::optional<std::string>& status,
                            const std::optional<std::string>& role,
                            const std::optional<std::string>& milestone_id,
                            const std::optional<int>& sort_order) {
    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (title.has_value()) {
        set_parts.push_back("title = ?");
        params.push_back(*title);
    }
    if (description.has_value()) {
        set_parts.push_back("description = ?");
        params.push_back(*description);
    }
    if (status.has_value()) {
        set_parts.push_back("status = ?");
        params.push_back(*status);
    }
    if (role.has_value()) {
        set_parts.push_back("role = ?");
        params.push_back(*role);
    }
    if (milestone_id.has_value()) {
        set_parts.push_back("milestone_id = ?");
        params.push_back(*milestone_id);
    }
    if (sort_order.has_value()) {
        set_parts.push_back("sort_order = ?");
        params.push_back(std::to_string(*sort_order));
    }

    if (set_parts.empty()) {
        return true; // Rien à mettre à jour
    }

    set_parts.push_back("updated_at = datetime('now')");

    std::string sql = "UPDATE tasks SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    return executor_.run(sql.c_str(), params);
}

bool TaskRepository::add_dependency(const std::string& task_id, const std::string& depends_on) {
    // Vérifier que la dépendance n'existe pas déjà
    auto rows_exist = executor_.query(
        "SELECT 1 FROM task_deps WHERE task_id = ? AND depends_on = ?",
        {task_id, depends_on});
    if (!rows_exist.empty()) {
        std::cerr << "taskman: dependency already exists\n";
        return false;
    }
    return executor_.run("INSERT INTO task_deps (task_id, depends_on) VALUES (?, ?)", {task_id, depends_on});
}

bool TaskRepository::remove_dependency(const std::string& task_id, const std::string& depends_on) {
    return executor_.run("DELETE FROM task_deps WHERE task_id = ? AND depends_on = ?", {task_id, depends_on});
}

bool TaskRepository::exists(const std::string& id) {
    auto rows = executor_.query("SELECT 1 FROM tasks WHERE id = ?", {id});
    return !rows.empty();
}

std::vector<std::map<std::string, std::optional<std::string>>> TaskRepository::get_dependencies(const std::string& task_id) {
    return executor_.query(
        "SELECT task_id, depends_on FROM task_deps WHERE task_id = ? ORDER BY depends_on",
        {task_id});
}

std::vector<std::map<std::string, std::optional<std::string>>> TaskRepository::list_dependencies(
    const std::optional<std::string>& task_id,
    int limit,
    int offset) {
    std::string sql = "SELECT task_id, depends_on FROM task_deps";
    std::vector<std::optional<std::string>> params;
    
    if (task_id.has_value()) {
        sql += " WHERE task_id = ?";
        params.push_back(*task_id);
    }
    
    sql += " ORDER BY task_id, depends_on LIMIT ? OFFSET ?";
    params.push_back(std::to_string(limit));
    params.push_back(std::to_string(offset));
    
    return executor_.query(sql.c_str(), params);
}

} // namespace taskman
