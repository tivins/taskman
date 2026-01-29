/**
 * Implémentation de l'exécuteur d'outils MCP.
 */

#include "mcp_tool_executor.hpp"
#include "cli/command.hpp"
#include "infrastructure/db/db.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

namespace taskman {

McpToolExecutor::McpToolExecutor(const McpToolRegistry& registry, const CommandRegistry& command_registry,
                                 const std::string& db_path)
    : tool_registry_(registry), command_registry_(command_registry), db_path_(db_path) {
}

std::string McpToolExecutor::json_to_string(const nlohmann::json& val) const {
    if (val.is_string()) {
        return val.get<std::string>();
    } else if (val.is_number_integer()) {
        return std::to_string(val.get<int>());
    } else if (val.is_number_unsigned()) {
        return std::to_string(val.get<unsigned>());
    } else if (val.is_number_float()) {
        return std::to_string(val.get<double>());
    } else if (val.is_boolean()) {
        return val.get<bool>() ? "1" : "0";
    } else if (val.is_null()) {
        return "";
    } else {
        return val.dump();
    }
}

std::vector<std::string> McpToolExecutor::build_argv(const McpToolDefinition& tool, const nlohmann::json& arguments) const {
    std::vector<std::string> argv;

    // D'abord les positionnels dans l'ordre
    for (const auto& key : tool.positional_keys) {
        if (arguments.contains(key) && !arguments[key].is_null()) {
            std::string val = json_to_string(arguments[key]);
            if (!val.empty()) {
                argv.push_back(val);
            }
        }
    }

    // Ensuite les options nommées (--key value)
    // On parcourt toutes les propriétés de l'inputSchema pour savoir quelles options sont valides
    if (tool.inputSchema.contains("properties") && tool.inputSchema["properties"].is_object()) {
        for (auto& [key, prop] : tool.inputSchema["properties"].items()) {
            // Ignorer les clés qui sont positionnelles (déjà ajoutées)
            bool is_positional = false;
            for (const auto& pos_key : tool.positional_keys) {
                if (key == pos_key) {
                    is_positional = true;
                    break;
                }
            }
            if (is_positional) continue;

            // Si la clé est présente dans arguments et non nulle, ajouter --key value
            if (arguments.contains(key) && !arguments[key].is_null()) {
                std::string val = json_to_string(arguments[key]);
                // Pour reached, convertir bool/int en "0"/"1"
                if (key == "reached") {
                    if (arguments[key].is_boolean()) {
                        val = arguments[key].get<bool>() ? "1" : "0";
                    } else if (arguments[key].is_number_integer()) {
                        int n = arguments[key].get<int>();
                        val = (n == 0) ? "0" : "1";
                    }
                }
                if (!val.empty()) {
                    argv.push_back("--" + key);
                    argv.push_back(val);
                }
            }
        }
    }

    return argv;
}

McpToolExecutor::CaptureGuard::CaptureGuard() {
    old_out = std::cout.rdbuf(oss_out.rdbuf());
    old_err = std::cerr.rdbuf(oss_err.rdbuf());
}

McpToolExecutor::CaptureGuard::~CaptureGuard() {
    std::cout.rdbuf(old_out);
    std::cerr.rdbuf(old_err);
}

int McpToolExecutor::execute_tool(const std::string& mcp_tool_name, const nlohmann::json& arguments,
                                   std::string& output, bool& is_error) {
    // Obtenir la définition de l'outil
    const McpToolDefinition* tool = tool_registry_.get_tool(mcp_tool_name);
    if (!tool) {
        return -1; // Outil inconnu
    }

    // Obtenir la commande CLI correspondante
    std::string cli_command = tool_registry_.get_cli_command(mcp_tool_name);
    if (cli_command.empty()) {
        output = "No CLI command mapped for tool: " + mcp_tool_name;
        is_error = true;
        return 1;
    }

    // Construire argv
    std::vector<std::string> argv_strs = build_argv(*tool, arguments);

    // Convertir en char*[] pour CommandRegistry
    // cxxopts peut s'attendre à argv[0] (nom de la commande), donc on ajoute un élément factice
    std::vector<char*> argv_ptrs;
    std::string dummy_argv0 = "taskman";
    argv_ptrs.push_back(const_cast<char*>(dummy_argv0.c_str()));
    for (auto& s : argv_strs) {
        argv_ptrs.push_back(const_cast<char*>(s.c_str()));
    }

    // Ouvrir la DB seulement si la commande en a besoin (évite de créer une DB vide pour rules/agents:generate)
    Database db;
    Database* db_ptr = nullptr;
    if (command_registry_.command_requires_database(cli_command)) {
        if (!db.open(db_path_.c_str())) {
            output = "Failed to open database";
            is_error = true;
            return 1;
        }
        db_ptr = &db;
    }

    // Cas spécial pour demo:generate qui doit fermer la DB avant de supprimer le fichier
    bool needs_db_close = (cli_command == "demo:generate");

    // Capture stdout/stderr
    CaptureGuard guard;
    int exit_code = 0;

    try {
        if (needs_db_close && db_ptr) {
            db.close();
        }

        // Exécuter via CommandRegistry
        exit_code = command_registry_.execute(cli_command, static_cast<int>(argv_ptrs.size()),
                                              argv_ptrs.data(), db_ptr);

        if (exit_code == -1) {
            // Commande non trouvée dans le registre
            output = "Command not found in registry: " + cli_command;
            is_error = true;
            return 1;
        }
    } catch (...) {
        exit_code = 1;
    }

    // Récupérer la sortie
    output = guard.get_out();
    std::string err_str = guard.get_err();
    if (!err_str.empty()) {
        if (!output.empty()) {
            output += "\n";
        }
        output += err_str;
    }

    is_error = (exit_code != 0);
    return exit_code;
}

} // namespace taskman
