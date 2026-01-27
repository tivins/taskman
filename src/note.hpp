/**
 * Commandes task:note:add, task:note:list.
 */

#ifndef TASKMAN_NOTE_HPP
#define TASKMAN_NOTE_HPP

#include <optional>
#include <string>

namespace taskman {

class Database;

/** Add a note to a task (reusable by CLI and programmatic code). */
bool note_add(Database& db,
              const std::string& id,
              const std::string& task_id,
              const std::string& content,
              const std::optional<std::string>& kind,
              const std::optional<std::string>& role);

/** task:note:add <task-id> --content "..." [--kind completion|progress|issue] [--role <role>] [--format json|text] */
int cmd_note_add(int argc, char* argv[], Database& db);

/** task:note:list <task-id> [--format json|text] */
int cmd_note_list(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_NOTE_HPP */
