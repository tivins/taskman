/**
 * PhaseRepository — accès à la base de données pour les phases uniquement.
 * Responsabilité unique : opérations CRUD sur la table phases.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_PHASE_REPOSITORY_HPP
#define TASKMAN_PHASE_REPOSITORY_HPP

#include "query_executor.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class PhaseRepository {
public:
    /** Constructeur prenant une référence à QueryExecutor. */
    explicit PhaseRepository(QueryExecutor& executor) : executor_(executor) {}

    PhaseRepository(const PhaseRepository&) = delete;
    PhaseRepository& operator=(const PhaseRepository&) = delete;

    /** Récupère une phase par son ID.
     * Retourne un map vide si la phase n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_by_id(const std::string& id);

    /** Liste les phases avec pagination.
     * Retourne un vecteur de maps représentant les phases. */
    std::vector<std::map<std::string, std::optional<std::string>>> list(int limit = 30, int offset = 0);

    /** Vérifie si une phase existe.
     * Retourne true si la phase existe, false sinon. */
    bool exists(const std::string& id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_PHASE_REPOSITORY_HPP */
