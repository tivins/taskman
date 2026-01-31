/**
 * NoteRepository — accès à la base de données pour les notes de tâches uniquement.
 * Responsabilité unique : opérations CRUD sur la table task_notes.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_NOTE_REPOSITORY_HPP
#define TASKMAN_NOTE_REPOSITORY_HPP

#include "infrastructure/db/query_executor.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class NoteRepository {
public:
    /** Constructeur prenant une référence à QueryExecutor. */
    explicit NoteRepository(QueryExecutor& executor) : executor_(executor) {}

    NoteRepository(const NoteRepository&) = delete;
    NoteRepository& operator=(const NoteRepository&) = delete;

    /** Insère une nouvelle note dans la base de données.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add(const std::string& id,
             const std::string& task_id,
             const std::string& content,
             const std::optional<std::string>& kind,
             const std::optional<std::string>& role);

    /** Récupère une note par son ID.
     * Retourne un map vide si la note n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_by_id(const std::string& id);

    /** Liste les notes d'une tâche.
     * Retourne un vecteur de maps représentant les notes. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_by_task_id(const std::string& task_id);

    /** Liste les notes dont les ID sont dans la liste fournie.
     * Retourne un vecteur de maps (ordre par created_at). Les IDs inexistants sont ignorés. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_by_ids(const std::vector<std::string>& ids);

    /** Vérifie si une tâche existe (pour validation avant d'ajouter une note).
     * Retourne true si la tâche existe, false sinon. */
    bool task_exists(const std::string& task_id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_NOTE_REPOSITORY_HPP */
