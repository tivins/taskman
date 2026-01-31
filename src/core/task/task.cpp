/**
 * Implémentation task:add, task:get, task:list, task:edit.
 * 
 * Ce fichier utilise maintenant les classes séparées selon le principe SRP:
 * - TaskRepository : accès DB
 * - TaskService : logique métier
 * - TaskFormatter : formatage sortie
 * - TaskCommandParser : parsing CLI
 * 
 * Les fonctions cmd_task_* sont des wrappers qui maintiennent la compatibilité
 * avec le code existant (main.cpp, mcp.cpp).
 */

#include "task.hpp"
#include "task_repository.hpp"
#include "task_service.hpp"
#include "task_formatter.hpp"
#include "task_command_parser.hpp"
#include "infrastructure/db/db.hpp"
#include "infrastructure/db/query_executor.hpp"
#include <memory>

namespace taskman {

bool task_add(Database& db,
              const std::string& id,
              const std::string& phase_id,
              const std::optional<std::string>& milestone_id,
              const std::string& title,
              const std::optional<std::string>& description,
              const std::string& status,
              std::optional<int> sort_order,
              const std::optional<std::string>& role,
              const std::optional<std::string>& creator) {
    // Utilise les nouvelles classes pour respecter le SRP
    // Passe par TaskService pour la validation
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    return service.add_task_with_id(id, phase_id, milestone_id, title, description, status, sort_order, role, creator);
}

bool task_dep_add(Database& db, const std::string& task_id, const std::string& depends_on) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    return service.add_task_dependency(task_id, depends_on);
}

int cmd_task_add(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_add(argc, argv);
}

int cmd_task_get(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_get(argc, argv);
}

int cmd_task_list(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_list(argc, argv);
}

int cmd_task_edit(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_edit(argc, argv);
}

int cmd_task_dep_add(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_dep_add(argc, argv);
}

int cmd_task_dep_remove(int argc, char* argv[], Database& db) {
    // Utilise les nouvelles classes pour respecter le SRP
    QueryExecutor& executor = db.get_executor();
    TaskRepository repository(executor);
    TaskService service(repository);
    TaskFormatter formatter;
    TaskCommandParser parser(service, formatter);
    return parser.parse_dep_remove(argc, argv);
}

} // namespace taskman
