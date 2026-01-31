/**
 * NoteCommandParser — parsing des arguments CLI pour les commandes de notes uniquement.
 * Responsabilité unique : parser les arguments CLI et appeler les services appropriés.
 * Utilise NoteService pour la logique métier et NoteFormatter pour le formatage.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_NOTE_COMMAND_PARSER_HPP
#define TASKMAN_NOTE_COMMAND_PARSER_HPP

#include "note_service.hpp"
#include "note_formatter.hpp"
#include <string>

namespace taskman {

class NoteCommandParser {
public:
    /** Constructeur prenant des références à NoteService et NoteFormatter. */
    NoteCommandParser(NoteService& service, NoteFormatter& formatter)
        : service_(service), formatter_(formatter) {}

    NoteCommandParser(const NoteCommandParser&) = delete;
    NoteCommandParser& operator=(const NoteCommandParser&) = delete;

    /** Parse et exécute la commande task:note:add.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_add(int argc, char* argv[]);

    /** Parse et exécute la commande task:note:list.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_list(int argc, char* argv[]);

    /** Parse et exécute la commande task:note:list-by-ids.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_list_by_ids(int argc, char* argv[]);

private:
    NoteService& service_;
    NoteFormatter& formatter_;
};

} // namespace taskman

#endif /* TASKMAN_NOTE_COMMAND_PARSER_HPP */
