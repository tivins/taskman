/**
 * NoteRepository — accès à la base de données pour les notes de tâches uniquement.
 * Responsabilité unique : opérations CRUD sur la table task_notes.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_NOTE_REPOSITORY_HPP
#define TASKMAN_NOTE_REPOSITORY_HPP

#include "query_executor.hpp"
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

    /** Liste les notes d'une tâche.
     * Retourne un vecteur de maps représentant les notes. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_by_task_id(const std::string& task_id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_NOTE_REPOSITORY_HPP */
