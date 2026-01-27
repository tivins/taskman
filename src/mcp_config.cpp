/**
 * Implémentation mcp:config.
 */

#include "mcp_config.hpp"
#include <algorithm>
#include <cxxopts.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace taskman {

int cmd_mcp_config(int argc, char* argv[]) {
    cxxopts::Options opts("taskman mcp:config", "Generate or update .cursor/mcp.json file");
    opts.add_options()
        ("executable", "Absolute path to taskman executable (required)", cxxopts::value<std::string>())
        ("output", "Output file path (default: .cursor/mcp.json)", cxxopts::value<std::string>()->default_value(".cursor/mcp.json"))
        ("help", "Show help");

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << '\n';
            std::cout << "\nExample:\n"
                      << "  taskman mcp:config --executable C:\\path\\to\\taskman.exe\n"
                      << "  taskman mcp:config --executable /usr/local/bin/taskman --output ~/.cursor/mcp.json\n";
            return 0;
        }
    }

    cxxopts::ParseResult result;
    try {
        result = opts.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    if (!result.count("executable")) {
        std::cerr << "taskman: --executable is required\n";
        std::cerr << opts.help() << '\n';
        return 1;
    }

    std::string executable_path = result["executable"].as<std::string>();
    std::string output_file = result["output"].as<std::string>();

    try {
        // Normalize executable path (convert to absolute if needed)
        fs::path exec_path(executable_path);
        if (!exec_path.is_absolute()) {
            std::cerr << "taskman: executable path must be absolute: " << executable_path << "\n";
            return 1;
        }

        // Get current working directory for TASKMAN_DB_NAME
        fs::path current_dir = fs::current_path();
        fs::path db_path = current_dir / "project_tasks.db";
        std::string db_path_str = db_path.string();
        
        // Normalize path separators for Windows (use forward slashes in JSON)
        std::string db_path_normalized = db_path_str;
        std::replace(db_path_normalized.begin(), db_path_normalized.end(), '\\', '/');
        
        std::string exec_path_normalized = executable_path;
        std::replace(exec_path_normalized.begin(), exec_path_normalized.end(), '\\', '/');

        // Create output directory if it doesn't exist
        fs::path output_path(output_file);
        fs::create_directories(output_path.parent_path());

        // Read existing mcp.json if it exists
        nlohmann::json mcp_config;
        if (fs::exists(output_path)) {
            {
                std::ifstream in_file(output_path.string());
                if (in_file.is_open()) {
                    try {
                        in_file >> mcp_config;
                    } catch (const nlohmann::json::exception& e) {
                        std::cerr << "taskman: error parsing existing " << output_file << ": " << e.what() << "\n";
                        return 1;
                    }
                }
            } // Explicitly close the file here
        }

        // Ensure mcpServers object exists
        if (!mcp_config.contains("mcpServers") || !mcp_config["mcpServers"].is_object()) {
            mcp_config["mcpServers"] = nlohmann::json::object();
        }

        // Create or update taskman server configuration
        nlohmann::json taskman_config;
        taskman_config["command"] = exec_path_normalized;
        taskman_config["args"] = nlohmann::json::array({"mcp"});
        taskman_config["env"] = nlohmann::json::object();
        taskman_config["env"]["TASKMAN_DB_NAME"] = db_path_normalized;
        taskman_config["env"]["TASKMAN_JOURNAL_MEMORY"] = "1";

        mcp_config["mcpServers"]["taskman"] = taskman_config;

        // Write the updated configuration
        std::ofstream out_file(output_path.string(), std::ios::trunc);
        if (!out_file) {
            std::cerr << "taskman: cannot write file: " << output_file << "\n";
            return 1;
        }

        out_file << mcp_config.dump(2) << '\n';
        if (!out_file) {
            std::cerr << "taskman: error writing file: " << output_file << "\n";
            return 1;
        }

        out_file.close();
        std::cout << "Updated " << output_file << " with taskman MCP server configuration\n";
        std::cout << "  Executable: " << exec_path_normalized << "\n";
        std::cout << "  Database: " << db_path_normalized << "\n";
        return 0;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "taskman: filesystem error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "taskman: error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace taskman
