/**
 * Implémentation de PhaseService.
 */

#include "phase_service.hpp"
#include <iostream>

namespace taskman {

bool PhaseService::create_phase(const std::string& id,
                                const std::string& name,
                                const std::string& status,
                                std::optional<int> sort_order) {
    // Validation du statut
    if (!is_valid_status(status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    // Insertion dans la base
    return repository_.add(id, name, status, sort_order);
}

std::map<std::string, std::optional<std::string>> PhaseService::get_phase(const std::string& id) {
    return repository_.get_by_id(id);
}

std::vector<std::map<std::string, std::optional<std::string>>> PhaseService::list_phases() {
    // Utilise une limite élevée pour obtenir toutes les phases
    return repository_.list(10000, 0);
}

bool PhaseService::update_phase(const std::string& id,
                                 const std::optional<std::string>& name,
                                 const std::optional<std::string>& status,
                                 const std::optional<int>& sort_order) {
    // Validation du statut si fourni
    if (status.has_value() && !is_valid_status(*status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    return repository_.update(id, name, status, sort_order);
}

bool PhaseService::is_valid_status(const std::string& status) {
    const char* const STATUS_VALUES[] = {"to_do", "in_progress", "done"};
    for (const char* v : STATUS_VALUES) {
        if (status == v) return true;
    }
    return false;
}

} // namespace taskman
