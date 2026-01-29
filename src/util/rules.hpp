/**
 * Commande rules:generate pour générer les fichiers .cursor/rules/.
 */

#ifndef TASKMAN_RULES_HPP
#define TASKMAN_RULES_HPP

namespace taskman {

/** rules:generate [--output <dir>]
 *  Génère les fichiers rules dans .cursor/rules/ (ou le répertoire spécifié).
 *  Les fichiers sont extraits depuis les assets intégrés dans le binaire.
 */
int cmd_rules_generate(int argc, char* argv[]);

} // namespace taskman

#endif /* TASKMAN_RULES_HPP */
