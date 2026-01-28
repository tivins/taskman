/**
 * Registre des outils MCP.
 * Responsabilité unique : définition des outils MCP (schémas, descriptions) et mapping vers les commandes CLI.
 * Respecte le principe SRP : séparation de la définition des outils de leur exécution.
 */

#ifndef TASKMAN_MCP_TOOL_REGISTRY_HPP
#define TASKMAN_MCP_TOOL_REGISTRY_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

namespace taskman {

/**
 * Structure représentant un outil MCP.
 */
struct McpToolDefinition {
    std::string name;                    // Nom de l'outil MCP (ex: "taskman_init")
    std::string cli_command;             // Commande CLI correspondante (ex: "init")
    std::string description;             // Description de l'outil
    nlohmann::json inputSchema;          // Schéma JSON pour les arguments
    std::vector<std::string> positional_keys;  // Clés qui doivent être passées comme arguments positionnels
};

/**
 * Registre des outils MCP.
 * Responsabilité : définition des outils MCP et mapping vers les commandes CLI.
 */
class McpToolRegistry {
public:
    McpToolRegistry();
    ~McpToolRegistry() = default;

    /**
     * Retourne la liste de tous les outils MCP au format JSON (pour tools/list).
     */
    std::vector<nlohmann::json> list_tools_json() const;

    /**
     * Retourne la définition d'un outil par son nom MCP.
     * @param mcp_name Nom de l'outil MCP (ex: "taskman_init")
     * @return Pointeur vers la définition, ou nullptr si non trouvé
     */
    const McpToolDefinition* get_tool(const std::string& mcp_name) const;

    /**
     * Retourne le nom de la commande CLI correspondant à un outil MCP.
     * @param mcp_name Nom de l'outil MCP
     * @return Nom de la commande CLI, ou chaîne vide si non trouvé
     */
    std::string get_cli_command(const std::string& mcp_name) const;

private:
    /**
     * Initialise le registre avec tous les outils MCP disponibles.
     */
    void initialize_tools();

    /**
     * Helper pour construire un inputSchema JSON.
     */
    nlohmann::json make_schema(const std::map<std::string, nlohmann::json>& props,
                               const std::vector<std::string>& required = {}) const;

    std::vector<McpToolDefinition> tools_;
    std::map<std::string, size_t> name_to_index_;  // Index rapide par nom MCP
};

} // namespace taskman

#endif /* TASKMAN_MCP_TOOL_REGISTRY_HPP */
