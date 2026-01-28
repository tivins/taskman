/**
 * Implémentation de NoteService.
 */

#include "note_service.hpp"
#include <iostream>
#include <random>
#include <uuid.h>

namespace taskman {

std::optional<std::string> NoteService::create_note(
    const std::string& task_id,
    const std::string& content,
    const std::optional<std::string>& kind,
    const std::optional<std::string>& role) {
    // Génération de l'ID
    std::string id = generate_uuid_v4();
    // Utilise la méthode avec ID spécifique
    if (!add_note_with_id(id, task_id, content, kind, role)) {
        return std::nullopt;
    }
    return id;
}

bool NoteService::add_note_with_id(
    const std::string& id,
    const std::string& task_id,
    const std::string& content,
    const std::optional<std::string>& kind,
    const std::optional<std::string>& role) {
    // Vérifier que la tâche existe
    if (!repository_.task_exists(task_id)) {
        std::cerr << "taskman: task not found: " << task_id << "\n";
        return false;
    }
    // Validation du rôle si fourni
    if (role.has_value() && !is_valid_role(*role)) {
        std::cerr << get_roles_error_message();
        return false;
    }
    // Insertion dans la base
    return repository_.add(id, task_id, content, kind, role);
}

std::map<std::string, std::optional<std::string>> NoteService::get_note(const std::string& id) {
    return repository_.get_by_id(id);
}

std::vector<std::map<std::string, std::optional<std::string>>> NoteService::list_notes(const std::string& task_id) {
    return repository_.list_by_task_id(task_id);
}

std::string NoteService::generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

} // namespace taskman
