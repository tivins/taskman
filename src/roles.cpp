/**
 * Implémentation des fonctions liées aux rôles.
 */

#include "roles.hpp"
#include <string>

namespace taskman {

const char* const ROLE_VALUES[] = {
    "project-manager", "project-designer", "software-architect",
    "developer", "summary-writer", "documentation-writer"};

const size_t ROLE_COUNT = sizeof(ROLE_VALUES) / sizeof(ROLE_VALUES[0]);

bool is_valid_role(const std::string& s) {
    for (const char* v : ROLE_VALUES) {
        if (s == v) return true;
    }
    return false;
}

nlohmann::json get_roles_json_array() {
    nlohmann::json arr = nlohmann::json::array();
    for (size_t i = 0; i < ROLE_COUNT; ++i) {
        arr.push_back(ROLE_VALUES[i]);
    }
    return arr;
}

std::string get_roles_error_message() {
    std::string msg = "taskman: --role must be one of: ";
    for (size_t i = 0; i < ROLE_COUNT; ++i) {
        if (i > 0) msg += ", ";
        msg += ROLE_VALUES[i];
    }
    msg += "\n";
    return msg;
}

} // namespace taskman
