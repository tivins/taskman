/**
 * Commande agents:generate pour générer les fichiers .cursor/agents/.
 */

#ifndef TASKMAN_AGENTS_HPP
#define TASKMAN_AGENTS_HPP

namespace taskman {

/** agents:generate [--output <dir>]
 *  Génère les fichiers agents dans .cursor/agents/ (ou le répertoire spécifié).
 *  Les fichiers sont extraits depuis les assets intégrés dans le binaire.
 */
int cmd_agents_generate(int argc, char* argv[]);

} // namespace taskman

#endif /* TASKMAN_AGENTS_HPP */
