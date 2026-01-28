/**
 * Commande mcp:config pour générer/modifier le fichier .cursor/mcp.json.
 */

#ifndef TASKMAN_MCP_CONFIG_HPP
#define TASKMAN_MCP_CONFIG_HPP

namespace taskman {

/** mcp:config <executable_path>
 *  Génère ou modifie le fichier .cursor/mcp.json avec la configuration du serveur MCP taskman.
 *  Prend en paramètre le chemin absolu de l'exécutable taskman (ou taskman.exe).
 *  Pour TASKMAN_DB_NAME, utilise le chemin courant + "project_tasks.db".
 */
int cmd_mcp_config(int argc, char* argv[]);

} // namespace taskman

#endif /* TASKMAN_MCP_CONFIG_HPP */
