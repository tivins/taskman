/**
 * MCP server — boucle stdio, parse JSON-RPC, dispatch (squelette + lifecycle).
 * Refactorisé selon SRP : utilisation de McpProtocolHandler, McpToolRegistry et McpToolExecutor.
 */

#include "mcp.hpp"
#include "mcp_protocol_handler.hpp"
#include "mcp_tool_registry.hpp"
#include "mcp_tool_executor.hpp"
#include "command.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace taskman {

namespace {

/** Obtient le chemin de la DB (identique à main.cpp). */
std::string get_db_path() {
    const char* env = std::getenv("TASKMAN_DB_NAME");
    return (env && env[0] != '\0') ? env : "project_tasks.db";
}

} // namespace

int run_mcp_server() {
    // Initialiser les composants
    McpProtocolHandler protocol_handler;
    McpToolRegistry tool_registry;
    
    // Créer le registre de commandes CLI
    CommandRegistry command_registry;
    register_all_commands(command_registry);
    
    // Créer l'exécuteur d'outils MCP
    McpToolExecutor tool_executor(tool_registry, command_registry, get_db_path());

    // Boucle principale : lire les requêtes JSON-RPC depuis stdin
    std::string line;
    while (std::getline(std::cin, line)) {
        // Traiter la requête via le protocole handler
        bool handled = protocol_handler.process_request(line);
        
        if (!handled) {
            // La requête nécessite un traitement spécial (tools/list ou tools/call)
            // On doit parser manuellement pour obtenir les détails
            nlohmann::json j;
            try {
                // Trim trailing \r (Windows CRLF)
                std::string trimmed_line = line;
                while (!trimmed_line.empty() && trimmed_line.back() == '\r')
                    trimmed_line.pop_back();
                j = nlohmann::json::parse(trimmed_line);
            } catch (const nlohmann::json::exception&) {
                // Déjà géré par process_request
                continue;
            }

            if (!j.contains("id") || !j.contains("method")) {
                continue;
            }

            nlohmann::json id = j["id"];
            std::string method = j["method"].get<std::string>();

            if (method == "tools/list") {
                // Obtenir la liste des outils depuis le registre
                auto tools_json = tool_registry.list_tools_json();
                protocol_handler.handle_tools_list(id, tools_json);
            } else if (method == "tools/call") {
                // Exécuter l'outil via l'exécuteur
                nlohmann::json params = j.contains("params") ? j["params"] : nlohmann::json();
                if (!params.is_object() || !params.contains("name")) {
                    continue; // Déjà validé par process_request
                }
                
                std::string tool_name = params["name"].get<std::string>();
                nlohmann::json arguments = params.contains("arguments") && params["arguments"].is_object()
                    ? params["arguments"] : nlohmann::json::object();

                std::string output;
                bool is_error = false;
                int result = tool_executor.execute_tool(tool_name, arguments, output, is_error);

                if (result == -1) {
                    // Outil inconnu → erreur JSON-RPC avec code -32602
                    protocol_handler.send_tool_error(id, "Unknown tool: " + tool_name);
                } else {
                    // Succès ou erreur métier → réponse normale
                    protocol_handler.handle_tools_call(id, params, output, is_error);
                }
            }
        }
    }
    return 0;
}

} // namespace taskman
