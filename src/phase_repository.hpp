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

    /** Insère une nouvelle phase dans la base de données.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool add(const std::string& id,
             const std::string& name,
             const std::string& status,
             std::optional<int> sort_order);

    /** Liste les phases avec pagination.
     * Retourne un vecteur de maps représentant les phases. */
    std::vector<std::map<std::string, std::optional<std::string>>> list(int limit = 30, int offset = 0);

    /** Met à jour une phase existante.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update(const std::string& id,
                const std::optional<std::string>& name = std::nullopt,
                const std::optional<std::string>& status = std::nullopt,
                const std::optional<int>& sort_order = std::nullopt);

    /** Vérifie si une phase existe.
     * Retourne true si la phase existe, false sinon. */
    bool exists(const std::string& id);

private:
    QueryExecutor& executor_;
};

} // namespace taskman

#endif /* TASKMAN_PHASE_REPOSITORY_HPP */
