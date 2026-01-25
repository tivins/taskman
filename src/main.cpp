/**
 * taskman — point d'entrée
 * Lit TASKMAN_DB_NAME (env), défaut project_tasks.db.
 * Dispatch argc/argv vers sous-commandes.
 * Codes de sortie : 0 = succès, 1 = erreur (parsing, DB, validation des args).
 */

#include "db.hpp"
#include "milestone.hpp"
#include "phase.hpp"
#include "task.hpp"
#include "version.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>

namespace {

const char* get_db_path() {
    const char* env = std::getenv("TASKMAN_DB_NAME");
    return (env && env[0] != '\0') ? env : "project_tasks.db";
}

struct CmdInfo {
    const char* name;
    const char* summary;
};

static const CmdInfo COMMANDS[] = {
    {"init",            "Create / initialize tables"},
    {"phase:add",       "Add a phase"},
    {"phase:edit",      "Edit a phase"},
    {"phase:list",      "List phases (JSON)"},
    {"milestone:add",   "Add a milestone"},
    {"milestone:edit",  "Edit a milestone"},
    {"milestone:list",  "List milestones (option --phase)"},
    {"task:add",        "Add a task (auto UUID)"},
    {"task:edit",       "Edit a task"},
    {"task:get",        "Get a task (JSON)"},
    {"task:list",       "List tasks (option --phase, --status, --role)"},
    {"task:dep:add",    "Add a dependency"},
    {"task:dep:remove", "Remove a dependency"},
};

static const std::size_t NUM_COMMANDS = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

std::string get_usage_string() {
    std::stringstream ss;
    ss << "\nTaskman " << TASKMAN_VERSION << "\n"
              << "https://github.com/tivins/taskman\n\n"
              << "Usage: taskman <command> [options]\n\n"
              << "Options:\n"
              << "  -h, --help      Show this help\n"
              << "  -v, --version   Show version\n"
              << "Commands:\n";
    for (std::size_t i = 0; i < NUM_COMMANDS; ++i) {
        ss << "  " << COMMANDS[i].name;
        const std::size_t len = std::strlen(COMMANDS[i].name);
        const std::size_t pad = (len <= 16) ? (16 - len) : 0;
        for (std::size_t k = 0; k < pad; ++k) ss << ' ';
        ss << " " << COMMANDS[i].summary << "\n";
    }
    ss << "\n"
              << "Environment variable:\n"
              << "  TASKMAN_DB_NAME (default: project_tasks.db)\n\n";
    return ss.str();
}


void print_usage() {
    std::cerr << get_usage_string() << "\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const char* cmd = argv[1];
    if (std::strcmp(cmd, "-v") == 0 || std::strcmp(cmd, "--version") == 0) {
        std::cout << "taskman " << TASKMAN_VERSION << "\n";
        return 0;
    }

    if (std::strcmp(cmd, "-h") == 0 || std::strcmp(cmd, "--help") == 0) {
        print_usage();
        return 0;
    }

    // Dispatch vers sous-commandes (implémentées en phases suivantes)
    if (std::strcmp(cmd, "init") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        if (!taskman::init_schema(db)) return 1;
        return 0;
    }
    if (std::strcmp(cmd, "phase:add") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_phase_add(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "phase:edit") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_phase_edit(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "phase:list") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_phase_list(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "milestone:add") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_milestone_add(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "milestone:edit") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_milestone_edit(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "milestone:list") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_milestone_list(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:add") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_add(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:edit") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_edit(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:get") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_get(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:list") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_list(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:dep:add") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_dep_add(argc - 1, argv + 1, db);
    }
    if (std::strcmp(cmd, "task:dep:remove") == 0) {
        taskman::Database db;
        if (!db.open(get_db_path())) return 1;
        return taskman::cmd_task_dep_remove(argc - 1, argv + 1, db);
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    print_usage();
    return 1;
}
