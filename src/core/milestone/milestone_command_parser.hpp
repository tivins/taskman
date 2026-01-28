/**
 * MilestoneCommandParser — parsing des arguments CLI pour les commandes de milestones uniquement.
 * Responsabilité unique : parser les arguments CLI et appeler les services appropriés.
 * Utilise MilestoneService pour la logique métier et MilestoneFormatter pour le formatage.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_MILESTONE_COMMAND_PARSER_HPP
#define TASKMAN_MILESTONE_COMMAND_PARSER_HPP

#include "milestone_service.hpp"
#include "milestone_formatter.hpp"
#include <string>

namespace taskman {

class MilestoneCommandParser {
public:
    /** Constructeur prenant des références à MilestoneService et MilestoneFormatter. */
    MilestoneCommandParser(MilestoneService& service, MilestoneFormatter& formatter)
        : service_(service), formatter_(formatter) {}

    MilestoneCommandParser(const MilestoneCommandParser&) = delete;
    MilestoneCommandParser& operator=(const MilestoneCommandParser&) = delete;

    /** Parse et exécute la commande milestone:add.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_add(int argc, char* argv[]);

    /** Parse et exécute la commande milestone:edit.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_edit(int argc, char* argv[]);

    /** Parse et exécute la commande milestone:list.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_list(int argc, char* argv[]);

private:
    MilestoneService& service_;
    MilestoneFormatter& formatter_;

    /** Parse un entier depuis une chaîne.
     * Retourne true si le parsing réussit, false sinon. */
    static bool parse_int(const std::string& s, int& out);

    /** Valide une valeur reached (0 ou 1).
     * Retourne true si valide, false sinon. */
    static bool is_valid_reached(const std::string& s);
};

} // namespace taskman

#endif /* TASKMAN_MILESTONE_COMMAND_PARSER_HPP */
