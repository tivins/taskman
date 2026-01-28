/**
 * PhaseCommandParser — parsing des arguments CLI pour les commandes de phases uniquement.
 * Responsabilité unique : parser les arguments CLI et appeler les services appropriés.
 * Utilise PhaseService pour la logique métier et PhaseFormatter pour le formatage.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_PHASE_COMMAND_PARSER_HPP
#define TASKMAN_PHASE_COMMAND_PARSER_HPP

#include "phase_service.hpp"
#include "phase_formatter.hpp"
#include <string>

namespace taskman {

class PhaseCommandParser {
public:
    /** Constructeur prenant des références à PhaseService et PhaseFormatter. */
    PhaseCommandParser(PhaseService& service, PhaseFormatter& formatter)
        : service_(service), formatter_(formatter) {}

    PhaseCommandParser(const PhaseCommandParser&) = delete;
    PhaseCommandParser& operator=(const PhaseCommandParser&) = delete;

    /** Parse et exécute la commande phase:add.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_add(int argc, char* argv[]);

    /** Parse et exécute la commande phase:edit.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_edit(int argc, char* argv[]);

    /** Parse et exécute la commande phase:list.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_list(int argc, char* argv[]);

private:
    PhaseService& service_;
    PhaseFormatter& formatter_;

    /** Parse un entier depuis une chaîne.
     * Retourne true si le parsing réussit, false sinon. */
    static bool parse_int(const std::string& s, int& out);
};

} // namespace taskman

#endif /* TASKMAN_PHASE_COMMAND_PARSER_HPP */
