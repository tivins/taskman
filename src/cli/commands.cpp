/**
 * Implémentations concrètes des commandes CLI utilisant le pattern Command.
 * Ces classes wrappent les fonctions existantes pour respecter l'interface Command.
 */

#include "command.hpp"
#include "infrastructure/db/db.hpp"
#include "core/task/task.hpp"
#include "core/phase/phase.hpp"
#include "core/milestone/milestone.hpp"
#include "core/note/note.hpp"
#include "util/demo.hpp"
#include "util/agents.hpp"
#include "util/rules.hpp"
#include "mcp/mcp_config.hpp"
#include "web/web.hpp"
#include "mcp/mcp.hpp"
#include <iostream>
#include <cstring>

namespace taskman {

// ============================================================================
// Commandes nécessitant une base de données
// ============================================================================

class InitCommand : public Command {
public:
    std::string name() const override { return "init"; }
    std::string summary() const override { return "Create / initialize tables"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        if (argc >= 2 && (std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0)) {
            std::cout << "taskman init\n\n"
                         "Create and initialize the database tables (phases, milestones, tasks, task_deps).\n"
                         "Run once when starting a new project.\n\n";
            return 0;
        }
        if (!db->init_schema()) return 1;
        return 0;
    }
};

class PhaseAddCommand : public Command {
public:
    std::string name() const override { return "phase:add"; }
    std::string summary() const override { return "Add a phase"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_phase_add(argc, argv, *db);
    }
};

class PhaseEditCommand : public Command {
public:
    std::string name() const override { return "phase:edit"; }
    std::string summary() const override { return "Edit a phase"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_phase_edit(argc, argv, *db);
    }
};

class PhaseListCommand : public Command {
public:
    std::string name() const override { return "phase:list"; }
    std::string summary() const override { return "List phases"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_phase_list(argc, argv, *db);
    }
};

class MilestoneAddCommand : public Command {
public:
    std::string name() const override { return "milestone:add"; }
    std::string summary() const override { return "Add a milestone"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_milestone_add(argc, argv, *db);
    }
};

class MilestoneEditCommand : public Command {
public:
    std::string name() const override { return "milestone:edit"; }
    std::string summary() const override { return "Edit a milestone"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_milestone_edit(argc, argv, *db);
    }
};

class MilestoneListCommand : public Command {
public:
    std::string name() const override { return "milestone:list"; }
    std::string summary() const override { return "List milestones (option --phase)"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_milestone_list(argc, argv, *db);
    }
};

class TaskAddCommand : public Command {
public:
    std::string name() const override { return "task:add"; }
    std::string summary() const override { return "Add a task (auto UUID)"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_add(argc, argv, *db);
    }
};

class TaskEditCommand : public Command {
public:
    std::string name() const override { return "task:edit"; }
    std::string summary() const override { return "Edit a task"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_edit(argc, argv, *db);
    }
};

class TaskGetCommand : public Command {
public:
    std::string name() const override { return "task:get"; }
    std::string summary() const override { return "Get a task"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_get(argc, argv, *db);
    }
};

class TaskListCommand : public Command {
public:
    std::string name() const override { return "task:list"; }
    std::string summary() const override { return "List tasks (option --phase, --status, --role)"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_list(argc, argv, *db);
    }
};

class TaskDepAddCommand : public Command {
public:
    std::string name() const override { return "task:dep:add"; }
    std::string summary() const override { return "Add a dependency"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_dep_add(argc, argv, *db);
    }
};

class TaskDepRemoveCommand : public Command {
public:
    std::string name() const override { return "task:dep:remove"; }
    std::string summary() const override { return "Remove a dependency"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_task_dep_remove(argc, argv, *db);
    }
};

class TaskNoteAddCommand : public Command {
public:
    std::string name() const override { return "task:note:add"; }
    std::string summary() const override { return "Add a note to a task"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_note_add(argc, argv, *db);
    }
};

class TaskNoteListCommand : public Command {
public:
    std::string name() const override { return "task:note:list"; }
    std::string summary() const override { return "List notes for a task"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_note_list(argc, argv, *db);
    }
};

class DemoGenerateCommand : public Command {
public:
    std::string name() const override { return "demo:generate"; }
    std::string summary() const override { return "Generate a demo database"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        // cmd_demo_generate gère l'ouverture/fermeture de la DB elle-même
        if (!db) return 1;
        return cmd_demo_generate(argc, argv, *db);
    }
};

class WebCommand : public Command {
public:
    std::string name() const override { return "web"; }
    std::string summary() const override { return "HTTP server for web UI (--host, --port)"; }
    
    int execute(int argc, char* argv[], Database* db) override {
        if (!db) return 1;
        return cmd_web(argc, argv, *db);
    }
};

// ============================================================================
// Commandes ne nécessitant pas de base de données
// ============================================================================

class AgentsGenerateCommand : public Command {
public:
    std::string name() const override { return "agents:generate"; }
    std::string summary() const override { return "Generate .cursor/agents/ files"; }
    bool requires_database() const override { return false; }
    
    int execute(int argc, char* argv[], Database* db) override {
        (void)db; // Non utilisé
        return cmd_agents_generate(argc, argv);
    }
};

class RulesGenerateCommand : public Command {
public:
    std::string name() const override { return "rules:generate"; }
    std::string summary() const override { return "Generate .cursor/rules/ files"; }
    bool requires_database() const override { return false; }
    
    int execute(int argc, char* argv[], Database* db) override {
        (void)db; // Non utilisé
        return cmd_rules_generate(argc, argv);
    }
};

class McpConfigCommand : public Command {
public:
    std::string name() const override { return "mcp:config"; }
    std::string summary() const override { return "Generate or update .cursor/mcp.json file"; }
    bool requires_database() const override { return false; }
    
    int execute(int argc, char* argv[], Database* db) override {
        (void)db; // Non utilisé
        return cmd_mcp_config(argc, argv);
    }
};

class McpCommand : public Command {
public:
    std::string name() const override { return "mcp"; }
    std::string summary() const override { return "MCP server (stdio): read JSON-RPC on stdin, write on stdout"; }
    bool requires_database() const override { return false; }
    
    int execute(int argc, char* argv[], Database* db) override {
        (void)argc;
        (void)argv;
        (void)db; // Non utilisé
        return run_mcp_server();
    }
};

// ============================================================================
// Fonction utilitaire pour initialiser le registre avec toutes les commandes
// ============================================================================

void register_all_commands(CommandRegistry& registry) {
    registry.register_command(std::make_unique<InitCommand>());
    registry.register_command(std::make_unique<PhaseAddCommand>());
    registry.register_command(std::make_unique<PhaseEditCommand>());
    registry.register_command(std::make_unique<PhaseListCommand>());
    registry.register_command(std::make_unique<MilestoneAddCommand>());
    registry.register_command(std::make_unique<MilestoneEditCommand>());
    registry.register_command(std::make_unique<MilestoneListCommand>());
    registry.register_command(std::make_unique<TaskAddCommand>());
    registry.register_command(std::make_unique<TaskEditCommand>());
    registry.register_command(std::make_unique<TaskGetCommand>());
    registry.register_command(std::make_unique<TaskListCommand>());
    registry.register_command(std::make_unique<TaskDepAddCommand>());
    registry.register_command(std::make_unique<TaskDepRemoveCommand>());
    registry.register_command(std::make_unique<TaskNoteAddCommand>());
    registry.register_command(std::make_unique<TaskNoteListCommand>());
    registry.register_command(std::make_unique<DemoGenerateCommand>());
    registry.register_command(std::make_unique<AgentsGenerateCommand>());
    registry.register_command(std::make_unique<RulesGenerateCommand>());
    registry.register_command(std::make_unique<McpConfigCommand>());
    registry.register_command(std::make_unique<McpCommand>());
    registry.register_command(std::make_unique<WebCommand>());
}

} // namespace taskman
