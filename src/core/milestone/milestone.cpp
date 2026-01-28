/**
 * Implémentation milestone:add, milestone:edit, milestone:list.
 * 
 * Ce fichier utilise maintenant les classes séparées selon le principe SRP:
 * - MilestoneRepository : accès DB
 * - MilestoneService : logique métier
 * - MilestoneFormatter : formatage sortie
 * - MilestoneCommandParser : parsing CLI
 * 
 * Les fonctions cmd_milestone_* sont des wrappers qui maintiennent la compatibilité
 * avec le code existant (main.cpp, mcp.cpp).
 */

#include "milestone.hpp"
#include "milestone_repository.hpp"
#include "milestone_service.hpp"
#include "milestone_formatter.hpp"
#include "milestone_command_parser.hpp"
#include "infrastructure/db/db.hpp"
#include "infrastructure/db/query_executor.hpp"
#include <memory>

namespace taskman {

bool milestone_add(Database& db,
                   const std::string& id,
                   const std::string& phase_id,
                   const std::optional<std::string>& name,
                   const std::optional<std::string>& criterion,
                   bool reached) {
    // Utilise les nouvelles classes pour respecter le SRP
    // Passe par MilestoneService pour la validation
    QueryExecutor& executor = db.get_executor();
    MilestoneRepository repository(executor);
    MilestoneService service(repository);
    return service.create_milestone(id, phase_id, name, criterion, reached);
}

int cmd_milestone_add(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    MilestoneRepository repository(executor);
    MilestoneService service(repository);
    MilestoneFormatter formatter;
    MilestoneCommandParser parser(service, formatter);
    return parser.parse_add(argc, argv);
}

int cmd_milestone_edit(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    MilestoneRepository repository(executor);
    MilestoneService service(repository);
    MilestoneFormatter formatter;
    MilestoneCommandParser parser(service, formatter);
    return parser.parse_edit(argc, argv);
}

int cmd_milestone_list(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    MilestoneRepository repository(executor);
    MilestoneService service(repository);
    MilestoneFormatter formatter;
    MilestoneCommandParser parser(service, formatter);
    return parser.parse_list(argc, argv);
}

} // namespace taskman
