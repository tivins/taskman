/**
 * Implémentation de NoteRepository.
 */

#include "note_repository.hpp"

namespace taskman {

bool NoteRepository::add(const std::string& id,
                         const std::string& task_id,
                         const std::string& content,
                         const std::optional<std::string>& kind,
                         const std::optional<std::string>& role) {
    const char* sql = "INSERT INTO task_notes (id, task_id, content, kind, role) VALUES (?, ?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params = {id, task_id, content, kind, role};
    if (!executor_.run(sql, params)) return false;
    // Update task.updated_at when a note is added
    return executor_.run("UPDATE tasks SET updated_at = datetime('now') WHERE id = ?", {task_id});
}

std::map<std::string, std::optional<std::string>> NoteRepository::get_by_id(const std::string& id) {
    auto rows = executor_.query(
        "SELECT id, task_id, content, kind, role, created_at FROM task_notes WHERE id = ?",
        {id});
    if (rows.empty()) {
        return {};
    }
    return rows[0];
}

std::vector<std::map<std::string, std::optional<std::string>>> NoteRepository::list_by_task_id(const std::string& task_id) {
    return executor_.query(
        "SELECT id, task_id, content, kind, role, created_at FROM task_notes WHERE task_id = ? ORDER BY created_at",
        {task_id});
}

bool NoteRepository::task_exists(const std::string& task_id) {
    auto rows = executor_.query("SELECT 1 FROM tasks WHERE id = ?", {task_id});
    return !rows.empty();
}

} // namespace taskman
