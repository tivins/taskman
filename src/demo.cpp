/**
 * Implémentation demo:generate — crée une base de données de démonstration.
 */

#include "demo.hpp"
#include "db.hpp"
#include "phase.hpp"
#include "milestone.hpp"
#include "task.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <uuid.h>

namespace {

std::string generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

} // namespace

namespace taskman {

int cmd_demo_generate(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman demo:generate", "Generate a demo database");
    opts.add_options()
        ("help", "Show help");

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "taskman demo:generate\n\n"
                         "Create a database filled with a real-life example (e-commerce site MVP project).\n"
                         "The database path is determined by TASKMAN_DB_NAME environment variable\n"
                         "or defaults to 'project_tasks.db'.\n\n"
                         "If the database already exists, it will be removed and recreated.\n\n";
            return 0;
        }
    }

    try {
        opts.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    // Get database path
    const char* db_path_env = std::getenv("TASKMAN_DB_NAME");
    std::string db_path = (db_path_env && db_path_env[0] != '\0') ? db_path_env : "project_tasks.db";

    // Ensure database is closed before we try to delete it
    db.close();

    // Remove existing database and related files for a reproducible demo
    namespace fs = std::filesystem;
    if (fs::exists(db_path)) {
        try {
            fs::remove(db_path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "taskman: cannot remove " << db_path << " (" << e.what() << "). Close any process using it.\n";
            return 1;
        }
    }
    // Remove journal files
    for (const std::string& suffix : {"-journal", "-wal", "-shm"}) {
        std::string path = db_path + suffix;
        if (fs::exists(path)) {
            try {
                fs::remove(path);
            } catch (const fs::filesystem_error&) {
                // Ignore errors for journal files
            }
        }
    }

    std::cout << "Creating demo database with a real-life example (e-commerce site MVP project)...\n";

    // Reopen database
    if (!db.open(db_path.c_str())) {
        std::cerr << "taskman: failed to open database\n";
        return 1;
    }

    // Initialize schema
    if (!init_schema(db)) {
        std::cerr << "taskman: failed to initialize schema\n";
        return 1;
    }
    std::cout << "  init\n";

    // Phases
    if (!phase_add(db, "P1", "Design", "in_progress", 1)) return 1;
    if (!phase_add(db, "P2", "Development", "to_do", 2)) return 1;
    if (!phase_add(db, "P3", "Acceptance", "to_do", 3)) return 1;
    if (!phase_add(db, "P4", "Delivery", "to_do", 4)) return 1;
    std::cout << "  phases P1-P4\n";

    // Milestones
    if (!milestone_add(db, "M1", "P1", "Specs approved", "Document signed off by client", true)) return 1;
    if (!milestone_add(db, "M2", "P2", "MVP delivered", "Catalog, cart and test payment operational", false)) return 1;
    if (!milestone_add(db, "M3", "P3", "Acceptance OK", "E2E tests passing, blocking bugs resolved", false)) return 1;
    if (!milestone_add(db, "M4", "P4", "Production deployment", "App deployed and reachable", false)) return 1;
    std::cout << "  milestones M1-M4\n";

    // Tasks (order compatible with dependencies)
    std::string t1 = generate_uuid_v4();
    if (!task_add(db, t1, "P1", "M1", "Write requirements document", std::nullopt, "done", 1, "project-manager")) return 1;

    std::string t2 = generate_uuid_v4();
    if (!task_add(db, t2, "P1", "M1", "Specify auth API", std::nullopt, "done", 2, "software-architect")) return 1;

    std::string t3 = generate_uuid_v4();
    if (!task_add(db, t3, "P1", "M1", "Validate specs", std::nullopt, "in_progress", 3, "project-designer")) return 1;

    std::string t4 = generate_uuid_v4();
    if (!task_add(db, t4, "P2", "M2", "Implement auth module (API)", "Endpoints login, logout, refresh", "in_progress", 4, "developer")) return 1;

    std::string t5 = generate_uuid_v4();
    if (!task_add(db, t5, "P2", "M2", "Implement login screen", std::nullopt, "to_do", 5, "developer")) return 1;

    std::string t6 = generate_uuid_v4();
    if (!task_add(db, t6, "P2", "M2", "Document auth API", std::nullopt, "to_do", 6, "documentation-writer")) return 1;

    std::string t7 = generate_uuid_v4();
    if (!task_add(db, t7, "P3", "M3", "Write E2E tests for login", std::nullopt, "to_do", 7, "developer")) return 1;

    std::string t8 = generate_uuid_v4();
    if (!task_add(db, t8, "P4", "M4", "Write deployment runbook", std::nullopt, "to_do", 8, "documentation-writer")) return 1;

    std::string t9 = generate_uuid_v4();
    if (!task_add(db, t9, "P4", "M4", "Deploy to production", std::nullopt, "to_do", 9, "project-manager")) return 1;
    std::cout << "  tasks 1-9\n";

    // Dependencies: 4->2, 5->4, 6->4, 7->5, 9->7, 9->8
    if (!task_dep_add(db, t4, t2)) return 1;
    if (!task_dep_add(db, t5, t4)) return 1;
    if (!task_dep_add(db, t6, t4)) return 1;
    if (!task_dep_add(db, t7, t5)) return 1;
    if (!task_dep_add(db, t9, t7)) return 1;
    if (!task_dep_add(db, t9, t8)) return 1;
    std::cout << "  dependencies\n";

    std::cout << "Done. Database: " << db_path << "\n";
    return 0;
}

} // namespace taskman
