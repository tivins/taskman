/**
 * taskman — point d'entrée
 * Lit TASKMAN_DB_NAME (env), défaut project_tasks.db.
 * Dispatch argc/argv vers sous-commandes via CommandRegistry (pattern Command).
 * Codes de sortie : 0 = succès, 1 = erreur (parsing, DB, validation des args).
 */

#include "command.hpp"
#include "infrastructure/db/db.hpp"
#include "version.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

namespace {

const char* get_db_path() {
    const char* env = std::getenv("TASKMAN_DB_NAME");
    return (env && env[0] != '\0') ? env : "project_tasks.db";
}

std::string get_usage_string(const taskman::CommandRegistry& registry) {
    std::stringstream ss;
    ss << "\nTaskman " << TASKMAN_VERSION << "\n"
              << "https://github.com/tivins/taskman\n\n"
              << "Usage: taskman <command> [options]\n\n"
              << "Options:\n"
              << "  -h, --help      Show this help\n"
              << "  -v, --version   Show version\n"
              << "Commands:\n";
    
    auto commands = registry.list_commands();
    for (const auto& [name, summary] : commands) {
        ss << "  " << name;
        const std::size_t len = name.length();
        const std::size_t pad = (len <= 16) ? (16 - len) : 0;
        for (std::size_t k = 0; k < pad; ++k) ss << ' ';
        ss << " " << summary << "\n";
    }
    ss << "\n"
              << "Use 'taskman <command> --help' for command-specific options.\n\n"
              << "Environment variables:\n"
              << "  TASKMAN_DB_NAME       (default: project_tasks.db)\n"
              << "  TASKMAN_JOURNAL_MEMORY=1  use in-memory journal (avoids disk I/O in sandboxes, e.g. Cursor agent)\n"
              << "  CURSOR_AGENT          when set, same as TASKMAN_JOURNAL_MEMORY=1\n\n";
    return ss.str();
}

void print_usage(const taskman::CommandRegistry& registry) {
    std::cerr << get_usage_string(registry) << "\n";
}

} // namespace

int main(int argc, char* argv[]) {
    // Initialiser le registre de commandes
    taskman::CommandRegistry registry;
    taskman::register_all_commands(registry);

    if (argc < 2) {
        print_usage(registry);
        return 1;
    }

    const char* cmd = argv[1];
    
    // Gestion des options globales
    if (std::strcmp(cmd, "-v") == 0 || std::strcmp(cmd, "--version") == 0) {
        std::cout << "taskman " << TASKMAN_VERSION << "\n";
        return 0;
    }

    if (std::strcmp(cmd, "-h") == 0 || std::strcmp(cmd, "--help") == 0) {
        print_usage(registry);
        return 0;
    }

    // Vérifier si la commande existe
    if (!registry.has_command(cmd)) {
        std::cerr << "Unknown command: " << cmd << "\n";
        print_usage(registry);
        return 1;
    }

    // Préparer la base de données si nécessaire
    std::unique_ptr<taskman::Database> db;
    taskman::Database* db_ptr = nullptr;
    
    if (registry.command_requires_database(cmd)) {
        db = std::make_unique<taskman::Database>();
        if (!db->open(get_db_path())) {
            return 1;
        }
        db_ptr = db.get();
    }

    // Exécuter la commande via le registre
    int result = registry.execute(cmd, argc - 1, argv + 1, db_ptr);
    
    if (result == -1) {
        std::cerr << "Unknown command: " << cmd << "\n";
        print_usage(registry);
        return 1;
    }

    return result;
}
