/**
 * MilestoneFormatter — formatage de sortie pour les milestones uniquement.
 * Responsabilité unique : convertir les données de milestones en différents formats (JSON, text).
 * Utilise les fonctions existantes de formats.hpp.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_MILESTONE_FORMATTER_HPP
#define TASKMAN_MILESTONE_FORMATTER_HPP

#include "formats.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace taskman {

class MilestoneFormatter {
public:
    /** Formate un milestone en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json(const std::map<std::string, std::optional<std::string>>& milestone, std::ostream& out);

    /** Formate un milestone en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text(const std::map<std::string, std::optional<std::string>>& milestone, std::ostream& out);

    /** Formate une liste de milestones en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& milestones, std::ostream& out);

    /** Formate une liste de milestones en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& milestones, std::ostream& out);

    /** Valide un format de sortie.
     * Retourne true si le format est valide (json ou text), false sinon. */
    static bool is_valid_format(const std::string& format);
};

} // namespace taskman

#endif /* TASKMAN_MILESTONE_FORMATTER_HPP */
