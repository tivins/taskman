/**
 * Définition centralisée des rôles disponibles dans le système.
 * Source unique de vérité pour la liste des rôles.
 */

#ifndef TASKMAN_ROLES_HPP
#define TASKMAN_ROLES_HPP

#include <cstddef>
#include <string>
#include <nlohmann/json.hpp>

namespace taskman {

/** Tableau des valeurs de rôles valides. */
extern const char* const ROLE_VALUES[];
extern const size_t ROLE_COUNT;

/** Vérifie si une chaîne est un rôle valide. */
bool is_valid_role(const std::string& s);

/** Retourne le tableau JSON des rôles (pour les schémas MCP). */
nlohmann::json get_roles_json_array();

/** Retourne le message d'erreur listant les rôles valides. */
std::string get_roles_error_message();

} // namespace taskman

#endif /* TASKMAN_ROLES_HPP */
