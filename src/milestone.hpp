/**
 * Commandes milestone:add, milestone:edit, milestone:list.
 */

#ifndef TASKMAN_MILESTONE_HPP
#define TASKMAN_MILESTONE_HPP

#include <optional>
#include <string>

namespace taskman {

class Database;

/** Add a milestone (reusable by CLI and programmatic code). */
bool milestone_add(Database& db,
                  const std::string& id,
                  const std::string& phase_id,
                  const std::optional<std::string>& name = std::nullopt,
                  const std::optional<std::string>& criterion = std::nullopt,
                  bool reached = false);

/** milestone:add --id <id> --phase <id> [--name ...] [--criterion ...] [--reached 0|1] */
int cmd_milestone_add(int argc, char* argv[], Database& db);

/** milestone:edit <id> [--name ...] [--criterion ...] [--reached 0|1] [--phase <id>] */
int cmd_milestone_edit(int argc, char* argv[], Database& db);

/** milestone:list [--phase <id>] → JSON tableau d'objets milestones */
int cmd_milestone_list(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_MILESTONE_HPP */
