/**
 * MCP (Model Context Protocol) server — mode stdio.
 * Lit les requêtes JSON-RPC sur stdin, écrit les réponses sur stdout.
 * Déclarations pour le squelette et le lifecycle (initialize, tools/list, tools/call).
 */

#ifndef TASKMAN_MCP_HPP
#define TASKMAN_MCP_HPP

namespace taskman {

/** Point d'entrée du mode `taskman mcp`. Boucle stdio jusqu'à EOF. Retourne 0. */
int run_mcp_server();

} // namespace taskman

#endif /* TASKMAN_MCP_HPP */
