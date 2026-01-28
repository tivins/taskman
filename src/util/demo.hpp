/**
 * Commande demo:generate — crée une base de données de démonstration.
 */

#ifndef TASKMAN_DEMO_HPP
#define TASKMAN_DEMO_HPP

namespace taskman {

class Database;

/** demo:generate [--db-path <path>] — crée une base de données remplie avec un exemple réaliste */
int cmd_demo_generate(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_DEMO_HPP */
