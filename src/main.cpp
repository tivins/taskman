/**
 * taskman — point d'entrée
 * Lit TASKMAN_DB_NAME (env), défaut project_tasks.db.
 * Dispatch argc/argv vers sous-commandes.
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {

const char* get_db_path() {
    const char* env = std::getenv("TASKMAN_DB_NAME");
    return (env && env[0] != '\0') ? env : "project_tasks.db";
}

void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <command> [options]\n"
              << "Commands: init, phase:add, phase:edit, phase:list,\n"
              << "          milestone:add, milestone:edit, milestone:list,\n"
              << "          task:add, task:edit, task:get, task:list,\n"
              << "          task:dep:add, task:dep:remove\n"
              << "Env: TASKMAN_DB_NAME (default: project_tasks.db)\n";
}

} // namespace

int main(int argc, char* argv[]) {
    [[maybe_unused]] const char* db_path = get_db_path(); // utilisé par la couche DB (phase 2)

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* cmd = argv[1];
    // Dispatch vers sous-commandes (implémentées en phases suivantes)
    if (std::strcmp(cmd, "init") == 0) {
        // taskman init
        std::cout << "init: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "phase:add") == 0) {
        std::cout << "phase:add: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "phase:edit") == 0) {
        std::cout << "phase:edit: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "phase:list") == 0) {
        std::cout << "phase:list: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "milestone:add") == 0) {
        std::cout << "milestone:add: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "milestone:edit") == 0) {
        std::cout << "milestone:edit: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "milestone:list") == 0) {
        std::cout << "milestone:list: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:add") == 0) {
        std::cout << "task:add: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:edit") == 0) {
        std::cout << "task:edit: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:get") == 0) {
        std::cout << "task:get: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:list") == 0) {
        std::cout << "task:list: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:dep:add") == 0) {
        std::cout << "task:dep:add: not implemented yet\n";
        return 0;
    }
    if (std::strcmp(cmd, "task:dep:remove") == 0) {
        std::cout << "task:dep:remove: not implemented yet\n";
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    print_usage(argv[0]);
    return 1;
}
