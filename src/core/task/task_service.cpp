/**
 * Implémentation de TaskService.
 */

#include "task_service.hpp"
#include "util/roles.hpp"
#include <iostream>
#include <random>
#include <uuid.h>

namespace taskman {

std::optional<std::string> TaskService::create_task(
    const std::string& phase_id,
    const std::optional<std::string>& milestone_id,
    const std::string& title,
    const std::optional<std::string>& description,
    const std::string& status,
    std::optional<int> sort_order,
    const std::optional<std::string>& role) {
    // Génération de l'ID
    std::string id = generate_uuid_v4();
    // Utilise la méthode avec ID spécifique
    if (!add_task_with_id(id, phase_id, milestone_id, title, description, status, sort_order, role)) {
        return std::nullopt;
    }
    return id;
}

bool TaskService::add_task_with_id(
    const std::string& id,
    const std::string& phase_id,
    const std::optional<std::string>& milestone_id,
    const std::string& title,
    const std::optional<std::string>& description,
    const std::string& status,
    std::optional<int> sort_order,
    const std::optional<std::string>& role) {
    // Validation du statut
    if (!is_valid_status(status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    // Validation du rôle si fourni
    if (role.has_value() && !is_valid_role(*role)) {
        std::cerr << get_roles_error_message();
        return false;
    }
    // Insertion dans la base
    return repository_.add(id, phase_id, milestone_id, title, description, status, sort_order, role);
}

std::map<std::string, std::optional<std::string>> TaskService::get_task(const std::string& id) {
    return repository_.get_by_id(id);
}

std::vector<std::map<std::string, std::optional<std::string>>> TaskService::list_tasks(
    const std::optional<std::string>& phase_id,
    const std::optional<std::string>& status,
    const std::optional<std::string>& role) {
    return repository_.list(phase_id, status, role);
}

bool TaskService::update_task(const std::string& id,
                               const std::optional<std::string>& title,
                               const std::optional<std::string>& description,
                               const std::optional<std::string>& status,
                               const std::optional<std::string>& role,
                               const std::optional<std::string>& milestone_id,
                               const std::optional<int>& sort_order) {
    // Validation du statut si fourni
    if (status.has_value() && !is_valid_status(*status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    // Validation du rôle si fourni
    if (role.has_value() && !is_valid_role(*role)) {
        std::cerr << get_roles_error_message();
        return false;
    }
    return repository_.update(id, title, description, status, role, milestone_id, sort_order);
}

bool TaskService::add_task_dependency(const std::string& task_id, const std::string& depends_on) {
    // Vérifier qu'une tâche ne dépend pas d'elle-même
    if (task_id == depends_on) {
        std::cerr << "taskman: a task cannot depend on itself\n";
        return false;
    }
    // Vérifier que les deux tâches existent
    if (!repository_.exists(task_id)) {
        std::cerr << "taskman: task not found: " << task_id << "\n";
        return false;
    }
    if (!repository_.exists(depends_on)) {
        std::cerr << "taskman: task not found: " << depends_on << "\n";
        return false;
    }
    return repository_.add_dependency(task_id, depends_on);
}

bool TaskService::remove_task_dependency(const std::string& task_id, const std::string& depends_on) {
    return repository_.remove_dependency(task_id, depends_on);
}

bool TaskService::is_valid_status(const std::string& status) {
    const char* const STATUS_VALUES[] = {"to_do", "in_progress", "done"};
    for (const char* v : STATUS_VALUES) {
        if (status == v) return true;
    }
    return false;
}

std::string TaskService::generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

} // namespace taskman
