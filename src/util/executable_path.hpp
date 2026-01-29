/**
 * Chemin absolu de l'exécutable en cours.
 * Utilisé par project:init pour mcp:config sans exiger --executable.
 */

#ifndef TASKMAN_EXECUTABLE_PATH_HPP
#define TASKMAN_EXECUTABLE_PATH_HPP

#include <string>

namespace taskman {

/**
 * Retourne le chemin absolu de l'exécutable en cours d'exécution.
 * Sous Windows : GetModuleFileName.
 * Sous Linux : readlink("/proc/self/exe").
 * Sous macOS : _NSGetExecutablePath.
 * @return Chemin absolu, ou chaîne vide en cas d'échec.
 */
std::string get_executable_path();

} // namespace taskman

#endif /* TASKMAN_EXECUTABLE_PATH_HPP */
