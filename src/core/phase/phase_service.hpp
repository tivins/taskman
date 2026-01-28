/**
 * PhaseService — logique métier pour les phases uniquement.
 * Responsabilité unique : règles métier, validation.
 * Utilise PhaseRepository pour l'accès aux données.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_PHASE_SERVICE_HPP
#define TASKMAN_PHASE_SERVICE_HPP

#include "phase_repository.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class PhaseService {
public:
    /** Constructeur prenant une référence à PhaseRepository. */
    explicit PhaseService(PhaseRepository& repository) : repository_(repository) {}

    PhaseService(const PhaseService&) = delete;
    PhaseService& operator=(const PhaseService&) = delete;

    /** Crée une nouvelle phase avec validation.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool create_phase(const std::string& id,
                      const std::string& name,
                      const std::string& status = "to_do",
                      std::optional<int> sort_order = std::nullopt);

    /** Récupère une phase par son ID.
     * Retourne un map vide si la phase n'existe pas. */
    std::map<std::string, std::optional<std::string>> get_phase(const std::string& id);

    /** Liste les phases.
     * Retourne un vecteur de maps représentant les phases. */
    std::vector<std::map<std::string, std::optional<std::string>>> list_phases();

    /** Met à jour une phase existante.
     * Effectue la validation des données avant mise à jour.
     * Retourne true en cas de succès, false en cas d'erreur. */
    bool update_phase(const std::string& id,
                     const std::optional<std::string>& name = std::nullopt,
                     const std::optional<std::string>& status = std::nullopt,
                     const std::optional<int>& sort_order = std::nullopt);

    /** Valide un statut de phase.
     * Retourne true si le statut est valide, false sinon. */
    static bool is_valid_status(const std::string& status);

private:
    PhaseRepository& repository_;
};

} // namespace taskman

#endif /* TASKMAN_PHASE_SERVICE_HPP */
