/**
 * Commandes phase:add, phase:edit, phase:list.
 */

#ifndef TASKMAN_PHASE_HPP
#define TASKMAN_PHASE_HPP

namespace taskman {

class Database;

/** phase:add --id <id> --name <name> [--status to_do|in_progress|done] [--sort-order <n>] */
int cmd_phase_add(int argc, char* argv[], Database& db);

/** phase:edit <id> [--name ...] [--status ...] [--sort-order <n>] */
int cmd_phase_edit(int argc, char* argv[], Database& db);

/** phase:list → JSON tableau d'objets phases, ORDER BY sort_order */
int cmd_phase_list(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_PHASE_HPP */
