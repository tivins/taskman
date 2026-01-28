/**
 * Implémentation de NoteRepository.
 */

#include "note_repository.hpp"

namespace taskman {

std::vector<std::map<std::string, std::optional<std::string>>> NoteRepository::list_by_task_id(const std::string& task_id) {
    return executor_.query(
        "SELECT id, task_id, content, kind, role, created_at FROM task_notes WHERE task_id = ? ORDER BY created_at",
        {task_id});
}

} // namespace taskman
