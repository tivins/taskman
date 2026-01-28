/**
 * TaskFormatter — formatage de sortie pour les tâches uniquement.
 * Responsabilité unique : convertir les données de tâches en différents formats (JSON, text).
 * Utilise les fonctions existantes de formats.hpp.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_TASK_FORMATTER_HPP
#define TASKMAN_TASK_FORMATTER_HPP

#include "util/formats.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace taskman {

class TaskFormatter {
public:
    /** Formate une tâche en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json(const std::map<std::string, std::optional<std::string>>& task, std::ostream& out);

    /** Formate une tâche en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text(const std::map<std::string, std::optional<std::string>>& task, std::ostream& out);

    /** Formate une liste de tâches en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& tasks, std::ostream& out);

    /** Formate une liste de tâches en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& tasks, std::ostream& out);

    /** Valide un format de sortie.
     * Retourne true si le format est valide (json ou text), false sinon. */
    static bool is_valid_format(const std::string& format);
};

} // namespace taskman

#endif /* TASKMAN_TASK_FORMATTER_HPP */
