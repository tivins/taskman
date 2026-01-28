/**
 * Implémentation task:note:add, task:note:list.
 * 
 * Ce fichier utilise maintenant les classes séparées selon le principe SRP:
 * - NoteRepository : accès DB
 * - NoteService : logique métier
 * - NoteFormatter : formatage sortie
 * - NoteCommandParser : parsing CLI
 * 
 * Les fonctions cmd_note_* sont des wrappers qui maintiennent la compatibilité
 * avec le code existant (main.cpp, mcp.cpp).
 */

#include "note.hpp"
#include "note_repository.hpp"
#include "note_service.hpp"
#include "note_formatter.hpp"
#include "note_command_parser.hpp"
#include "db.hpp"
#include "query_executor.hpp"
#include <memory>

namespace taskman {

bool note_add(Database& db,
              const std::string& id,
              const std::string& task_id,
              const std::string& content,
              const std::optional<std::string>& kind,
              const std::optional<std::string>& role) {
    // Utilise les nouvelles classes pour respecter le SRP
    // Passe par NoteService pour la validation
    QueryExecutor& executor = db.get_executor();
    NoteRepository repository(executor);
    NoteService service(repository);
    return service.add_note_with_id(id, task_id, content, kind, role);
}

int cmd_note_add(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    NoteRepository repository(executor);
    NoteService service(repository);
    NoteFormatter formatter;
    NoteCommandParser parser(service, formatter);
    return parser.parse_add(argc, argv);
}

int cmd_note_list(int argc, char* argv[], Database& db) {
    QueryExecutor& executor = db.get_executor();
    NoteRepository repository(executor);
    NoteService service(repository);
    NoteFormatter formatter;
    NoteCommandParser parser(service, formatter);
    return parser.parse_list(argc, argv);
}

} // namespace taskman
