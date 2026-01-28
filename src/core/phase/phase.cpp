/**
 * Implémentation phase:add, phase:edit, phase:list.
 * 
 * Ce fichier utilise maintenant les classes séparées selon le principe SRP:
 * - PhaseRepository : accès DB
 * - PhaseService : logique métier
 * - PhaseFormatter : formatage sortie
 * - PhaseCommandParser : parsing CLI
 * 
 * Les fonctions cmd_phase_* sont des wrappers qui maintiennent la compatibilité
 * avec le code existant (main.cpp, mcp.cpp).
 */

#include "phase.hpp"
#include "phase_repository.hpp"
#include "phase_service.hpp"
#include "phase_formatter.hpp"
#include "phase_command_parser.hpp"
#include "infrastructure/db/db.hpp"
#include "infrastructure/db/query_executor.hpp"
#include <memory>

namespace taskman {

bool phase_add(Database& db,
               const std::string& id,
               const std::string& name,
               const std::string& status,
               std::optional<int> sort_order) {
    // Utilise les nouvelles classes pour respecter le SRP
    // Passe par PhaseService pour la validation
    QueryExecutor& executor = db.get_executor();
    PhaseRepository repository(executor);
    PhaseService service(repository);
    return service.create_phase(id, name, status, sort_order);
}

int cmd_phase_add(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    PhaseRepository repository(executor);
    PhaseService service(repository);
    PhaseFormatter formatter;
    PhaseCommandParser parser(service, formatter);
    return parser.parse_add(argc, argv);
}

int cmd_phase_edit(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    PhaseRepository repository(executor);
    PhaseService service(repository);
    PhaseFormatter formatter;
    PhaseCommandParser parser(service, formatter);
    return parser.parse_edit(argc, argv);
}

int cmd_phase_list(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    PhaseRepository repository(executor);
    PhaseService service(repository);
    PhaseFormatter formatter;
    PhaseCommandParser parser(service, formatter);
    return parser.parse_list(argc, argv);
}

} // namespace taskman
