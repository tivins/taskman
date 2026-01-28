/**
 * Exécuteur d'outils MCP.
 * Responsabilité unique : conversion JSON → argv et exécution via CommandRegistry.
 * Respecte le principe SRP : séparation de l'exécution des outils de leur définition.
 */

#ifndef TASKMAN_MCP_TOOL_EXECUTOR_HPP
#define TASKMAN_MCP_TOOL_EXECUTOR_HPP

#include "mcp_tool_registry.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace taskman {

class CommandRegistry;
class Database;

/**
 * Exécuteur d'outils MCP.
 * Responsabilité : conversion JSON → argv et exécution via CommandRegistry.
 */
class McpToolExecutor {
public:
    /**
     * @param registry Registre des outils MCP
     * @param command_registry Registre des commandes CLI
     * @param db_path Chemin vers la base de données
     */
    McpToolExecutor(const McpToolRegistry& registry, const CommandRegistry& command_registry,
                    const std::string& db_path);
    ~McpToolExecutor() = default;

    /**
     * Exécute un outil MCP.
     * @param mcp_tool_name Nom de l'outil MCP (ex: "taskman_init")
     * @param arguments Arguments JSON de l'outil
     * @param output Sortie de l'exécution (stdout + stderr)
     * @param is_error Indique si l'exécution a échoué
     * @return Code de sortie (0 = succès, != 0 = erreur, -1 = outil inconnu)
     */
    int execute_tool(const std::string& mcp_tool_name, const nlohmann::json& arguments,
                     std::string& output, bool& is_error);

private:
    /**
     * Convertit une valeur JSON en string pour argv.
     */
    std::string json_to_string(const nlohmann::json& val) const;

    /**
     * Construit argv à partir des arguments JSON et de la définition de l'outil.
     */
    std::vector<std::string> build_argv(const McpToolDefinition& tool, const nlohmann::json& arguments) const;

    /**
     * Capture stdout/stderr pendant l'exécution d'une commande.
     */
    class CaptureGuard {
    public:
        CaptureGuard();
        ~CaptureGuard();
        std::string get_out() const { return oss_out.str(); }
        std::string get_err() const { return oss_err.str(); }
    private:
        std::ostringstream oss_out, oss_err;
        std::streambuf* old_out;
        std::streambuf* old_err;
    };

    const McpToolRegistry& tool_registry_;
    const CommandRegistry& command_registry_;
    std::string db_path_;
};

} // namespace taskman

#endif /* TASKMAN_MCP_TOOL_EXECUTOR_HPP */
