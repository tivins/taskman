/**
 * Commandes task:add, task:get, task:list, task:edit.
 */

#ifndef TASKMAN_TASK_HPP
#define TASKMAN_TASK_HPP

namespace taskman {

class Database;

/** task:add --title <title> --phase <id> [--description ...] [--role ...] [--milestone <id>] [--format json|text]
 *  Génère ID UUID v4 ; INSERT ; sortie = tâche créée (comme task:get). */
int cmd_task_add(int argc, char* argv[], Database& db);

/** task:get <id> [--format json|text] ; JSON par défaut. */
int cmd_task_get(int argc, char* argv[], Database& db);

/** task:list [--phase <id>] [--status <s>] [--role <r>] [--format json|text] */
int cmd_task_list(int argc, char* argv[], Database& db);

/** task:edit <id> [--title ...] [--description ...] [--status ...] [--role ...] [--milestone <id>] → UPDATE partiel */
int cmd_task_edit(int argc, char* argv[], Database& db);

/** task:dep:add <task-id> <dep-id> → INSERT dans task_deps ; vérifie existence des tâches. */
int cmd_task_dep_add(int argc, char* argv[], Database& db);

/** task:dep:remove <task-id> <dep-id> → DELETE. */
int cmd_task_dep_remove(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_TASK_HPP */
