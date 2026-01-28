/**
 * Implémentation du registre des outils MCP.
 */

#include "mcp_tool_registry.hpp"
#include "util/roles.hpp"
#include <algorithm>

namespace taskman {

McpToolRegistry::McpToolRegistry() {
    initialize_tools();
}

nlohmann::json McpToolRegistry::make_schema(const std::map<std::string, nlohmann::json>& props,
                                            const std::vector<std::string>& required) const {
    nlohmann::json schema;
    schema["type"] = "object";
    schema["properties"] = nlohmann::json::object();
    for (const auto& [key, val] : props) {
        schema["properties"][key] = val;
    }
    if (!required.empty()) {
        schema["required"] = required;
    }
    return schema;
}

void McpToolRegistry::initialize_tools() {
    tools_.clear();
    name_to_index_.clear();

    // taskman_init → init
    {
        McpToolDefinition t;
        t.name = "taskman_init";
        t.cli_command = "init";
        t.description = "Create and initialize the database tables (phases, milestones, tasks, task_deps). Run once when starting a new project.";
        t.inputSchema = make_schema({});
        t.inputSchema["additionalProperties"] = false;
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_phase_add → phase:add
    {
        McpToolDefinition t;
        t.name = "taskman_phase_add";
        t.cli_command = "phase:add";
        t.description = "Add a new phase to the project.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Unique phase identifier"}};
        props["name"] = nlohmann::json{{"type", "string"}, {"description", "Phase name"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}, {"description", "Phase status"}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this phase. Lower values appear first. IMPORTANT: Always set this to control display order. Use increments of 10 (e.g. 10, 20, 30) to allow easy insertion later."}};
        t.inputSchema = make_schema(props, {"id", "name"});
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_phase_edit → phase:edit
    {
        McpToolDefinition t;
        t.name = "taskman_phase_edit";
        t.cli_command = "phase:edit";
        t.description = "Edit an existing phase.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Phase ID to edit"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this phase. Lower values appear first."}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_phase_list → phase:list
    {
        McpToolDefinition t;
        t.name = "taskman_phase_list";
        t.cli_command = "phase:list";
        t.description = "List all phases in JSON format.";
        t.inputSchema = make_schema({});
        t.inputSchema["additionalProperties"] = false;
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_milestone_add → milestone:add
    {
        McpToolDefinition t;
        t.name = "taskman_milestone_add";
        t.cli_command = "milestone:add";
        t.description = "Add a new milestone to a phase.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Unique milestone identifier"}};
        props["phase"] = nlohmann::json{{"type", "string"}, {"description", "Parent phase ID"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["criterion"] = nlohmann::json{{"type", "string"}};
        props["reached"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"0", "1"})}, {"description", "0=not reached, 1=reached"}};
        t.inputSchema = make_schema(props, {"id", "phase"});
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_milestone_edit → milestone:edit
    {
        McpToolDefinition t;
        t.name = "taskman_milestone_edit";
        t.cli_command = "milestone:edit";
        t.description = "Edit an existing milestone.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Milestone ID to edit"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["criterion"] = nlohmann::json{{"type", "string"}};
        props["reached"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"0", "1"})}};
        props["phase"] = nlohmann::json{{"type", "string"}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_milestone_list → milestone:list
    {
        McpToolDefinition t;
        t.name = "taskman_milestone_list";
        t.cli_command = "milestone:list";
        t.description = "List milestones, optionally filtered by phase.";
        std::map<std::string, nlohmann::json> props;
        props["phase"] = nlohmann::json{{"type", "string"}, {"description", "Filter by phase ID"}};
        t.inputSchema = make_schema(props);
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_add → task:add
    {
        McpToolDefinition t;
        t.name = "taskman_task_add";
        t.cli_command = "task:add";
        t.description = "Add a new task to a phase.";
        std::map<std::string, nlohmann::json> props;
        props["title"] = nlohmann::json{{"type", "string"}};
        props["phase"] = nlohmann::json{{"type", "string"}};
        props["description"] = nlohmann::json{{"type", "string"}};
        props["role"] = nlohmann::json{{"type", "string"}, {"enum", get_roles_json_array()}};
        props["milestone"] = nlohmann::json{{"type", "string"}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this task within its phase. Lower values appear first. IMPORTANT: Always set this to control display order. Use increments of 10 (e.g. 10, 20, 30) to allow easy insertion later."}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}, {"default", "json"}};
        t.inputSchema = make_schema(props, {"title", "phase"});
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_get → task:get
    {
        McpToolDefinition t;
        t.name = "taskman_task_get";
        t.cli_command = "task:get";
        t.description = "Get a task by ID in JSON or text format.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Task UUID"}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_list → task:list
    {
        McpToolDefinition t;
        t.name = "taskman_task_list";
        t.cli_command = "task:list";
        t.description = "List tasks, optionally filtered by phase, status, or role.";
        std::map<std::string, nlohmann::json> props;
        props["phase"] = nlohmann::json{{"type", "string"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}};
        props["role"] = nlohmann::json{{"type", "string"}, {"enum", get_roles_json_array()}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}};
        t.inputSchema = make_schema(props);
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_edit → task:edit
    {
        McpToolDefinition t;
        t.name = "taskman_task_edit";
        t.cli_command = "task:edit";
        t.description = "Edit an existing task.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Task UUID to edit"}};
        props["title"] = nlohmann::json{{"type", "string"}};
        props["description"] = nlohmann::json{{"type", "string"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}};
        props["role"] = nlohmann::json{{"type", "string"}, {"enum", get_roles_json_array()}};
        props["milestone"] = nlohmann::json{{"type", "string"}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this task within its phase. Lower values appear first."}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_dep_add → task:dep:add
    {
        McpToolDefinition t;
        t.name = "taskman_task_dep_add";
        t.cli_command = "task:dep:add";
        t.description = "Add a dependency between two tasks.";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that depends on another"}};
        props["dep-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that must be completed first"}};
        t.inputSchema = make_schema(props, {"task-id", "dep-id"});
        t.positional_keys = {"task-id", "dep-id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_dep_remove → task:dep:remove
    {
        McpToolDefinition t;
        t.name = "taskman_task_dep_remove";
        t.cli_command = "task:dep:remove";
        t.description = "Remove a dependency between two tasks.";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that depends on another"}};
        props["dep-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that must be completed first"}};
        t.inputSchema = make_schema(props, {"task-id", "dep-id"});
        t.positional_keys = {"task-id", "dep-id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_note_add → task:note:add
    {
        McpToolDefinition t;
        t.name = "taskman_task_note_add";
        t.cli_command = "task:note:add";
        t.description = "Add a note to a task (e.g. completion summary, progress, or issue).";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task ID to attach the note to"}};
        props["content"] = nlohmann::json{{"type", "string"}, {"description", "Note content"}};
        props["kind"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"completion", "progress", "issue"})}, {"description", "Note kind: completion, progress, or issue"}};
        props["role"] = nlohmann::json{{"type", "string"}, {"enum", get_roles_json_array()}, {"description", "Role of the agent who added the note"}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}, {"default", "json"}};
        t.inputSchema = make_schema(props, {"task-id", "content"});
        t.positional_keys = {"task-id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_task_note_list → task:note:list
    {
        McpToolDefinition t;
        t.name = "taskman_task_note_list";
        t.cli_command = "task:note:list";
        t.description = "List notes for a task.";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task ID"}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}, {"default", "json"}};
        t.inputSchema = make_schema(props, {"task-id"});
        t.positional_keys = {"task-id"};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }

    // taskman_demo_generate → demo:generate
    {
        McpToolDefinition t;
        t.name = "taskman_demo_generate";
        t.cli_command = "demo:generate";
        t.description = "Generate a demo database filled with a realistic example (e-commerce site MVP project). Automatically removes any existing database file before creating the new one.";
        t.inputSchema = make_schema({});
        t.inputSchema["additionalProperties"] = false;
        t.positional_keys = {};
        tools_.push_back(t);
        name_to_index_[t.name] = tools_.size() - 1;
    }
}

std::vector<nlohmann::json> McpToolRegistry::list_tools_json() const {
    std::vector<nlohmann::json> result;
    for (const auto& tool : tools_) {
        nlohmann::json t;
        t["name"] = tool.name;
        t["description"] = tool.description;
        t["inputSchema"] = tool.inputSchema;
        result.push_back(t);
    }
    return result;
}

const McpToolDefinition* McpToolRegistry::get_tool(const std::string& mcp_name) const {
    auto it = name_to_index_.find(mcp_name);
    if (it == name_to_index_.end()) {
        return nullptr;
    }
    return &tools_[it->second];
}

std::string McpToolRegistry::get_cli_command(const std::string& mcp_name) const {
    const McpToolDefinition* tool = get_tool(mcp_name);
    return tool ? tool->cli_command : "";
}

} // namespace taskman
