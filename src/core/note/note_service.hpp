/**
 * NoteService — logique métier pour les notes de tâches uniquement.
 * Responsabilité unique : règles métier, validation, génération d'identifiants.
 * Utilise NoteRepository pour l'accès aux données.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_NOTE_SERVICE_HPP
#define TASKMAN_NOTE_SERVICE_HPP

#include "note_repository.hpp"
#include "util/roles.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class NoteService {
public:
    /** Constructeur prenant une référence à NoteRepository. */
    explicit NoteService(NoteRepository& repository) : repository_(repository) {}

    NoteService(const NoteService&) = delete;
    NoteService& operator=(const NoteService&) = delete;

    /** Crée une nouvelle note avec génération automatique d'ID UUID v4.
     * Effectue la validation des données avant insertion.
     * Retourne l'ID de la note créée, ou nullopt en cas d'erreur. */
    std::optional<std::string> create_note(
        const std::string& task_id,
        const std::string& content,
        const std::optional<std::string>& kind = std::nullopt,
        const std::optional<std::string>& role = std::nullopt);

    /** Ajoute une note avec un ID spécifique (pour compatibilité avec les tests et code existant).
     * Effectue la validation des données avant insertion.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add_note_with_id(
        const std::string& id,
        const std::string& task_id,
        const std::string& content,
        const std::optional<std::string>& kind = std::nullopt,
        const std::optional<std::string>& role = std::nullopt);

    /** Récupère une note par son ID.
     * Retourne un map vide si la note n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_note(const std::string& id);

    /** Liste les notes d'une tâche.
     * Retourne un vecteur de maps représentant les notes. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_notes(const std::string& task_id);

    /** Génère un UUID v4.
     * Retourne une chaîne représentant l'UUID. */
    static std::string generate_uuid_v4();

private:
    NoteRepository& repository_;
};

} // namespace taskman

#endif /* TASKMAN_NOTE_SERVICE_HPP */
