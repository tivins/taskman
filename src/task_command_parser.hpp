/**
 * TaskCommandParser — parsing des arguments CLI pour les commandes de tâches uniquement.
 * Responsabilité unique : parser les arguments CLI et appeler les services appropriés.
 * Utilise TaskService pour la logique métier et TaskFormatter pour le formatage.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_TASK_COMMAND_PARSER_HPP
#define TASKMAN_TASK_COMMAND_PARSER_HPP

#include "task_service.hpp"
#include "task_formatter.hpp"
#include <string>

namespace taskman {

class TaskCommandParser {
public:
    /** Constructeur prenant des références à TaskService et TaskFormatter. */
    TaskCommandParser(TaskService& service, TaskFormatter& formatter)
        : service_(service), formatter_(formatter) {}

    TaskCommandParser(const TaskCommandParser&) = delete;
    TaskCommandParser& operator=(const TaskCommandParser&) = delete;

    /** Parse et exécute la commande task:add.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_add(int argc, char* argv[]);

    /** Parse et exécute la commande task:get.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_get(int argc, char* argv[]);

    /** Parse et exécute la commande task:list.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_list(int argc, char* argv[]);

    /** Parse et exécute la commande task:edit.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_edit(int argc, char* argv[]);

    /** Parse et exécute la commande task:dep:add.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_dep_add(int argc, char* argv[]);

    /** Parse et exécute la commande task:dep:remove.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int parse_dep_remove(int argc, char* argv[]);

private:
    TaskService& service_;
    TaskFormatter& formatter_;

    /** Parse un entier depuis une chaîne.
     * Retourne true si le parsing réussit, false sinon. */
    static bool parse_int(const std::string& s, int& out);
};

} // namespace taskman

#endif /* TASKMAN_TASK_COMMAND_PARSER_HPP */
