/**
 * NoteFormatter — formatage de sortie pour les notes de tâches uniquement.
 * Responsabilité unique : convertir les données de notes en différents formats (JSON, text).
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_NOTE_FORMATTER_HPP
#define TASKMAN_NOTE_FORMATTER_HPP

#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace taskman {

class NoteFormatter {
public:
    /** Formate une note en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json(const std::map<std::string, std::optional<std::string>>& note, std::ostream& out);

    /** Formate une note en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text(const std::map<std::string, std::optional<std::string>>& note, std::ostream& out);

    /** Formate une liste de notes en JSON.
     * Écrit le résultat dans le stream fourni. */
    static void format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& notes, std::ostream& out);

    /** Formate une liste de notes en texte lisible.
     * Écrit le résultat dans le stream fourni. */
    static void format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& notes, std::ostream& out);

    /** Valide un format de sortie.
     * Retourne true si le format est valide (json ou text), false sinon. */
    static bool is_valid_format(const std::string& format);
};

} // namespace taskman

#endif /* TASKMAN_NOTE_FORMATTER_HPP */
