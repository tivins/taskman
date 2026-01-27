/**
 * MCP server — boucle stdio, parse JSON-RPC, dispatch (squelette + lifecycle).
 * Erreurs : -32700 Parse error, -32600 Invalid Request, -32601 Method not found,
 *           -32602 Unsupported protocol version.
 */

#include "mcp.hpp"
#include "db.hpp"
#include "milestone.hpp"
#include "phase.hpp"
#include "roles.hpp"
#include "task.hpp"
#include "version.h"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace taskman {

namespace {

struct McpTool {
    std::string name;
    std::string description;
    nlohmann::json inputSchema;
    std::vector<std::string> positional_keys;
};

/** Helper pour construire un inputSchema. */
nlohmann::json make_schema(const std::map<std::string, nlohmann::json>& props,
                            const std::vector<std::string>& required = {}) {
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

/** Table statique des 13 outils MCP. */
std::vector<McpTool> get_mcp_tools() {
    std::vector<McpTool> tools;

    // taskman_init
    {
        McpTool t;
        t.name = "taskman_init";
        t.description = "Create and initialize the database tables (phases, milestones, tasks, task_deps). Run once when starting a new project.";
        t.inputSchema = make_schema({});
        t.inputSchema["additionalProperties"] = false;
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_phase_add
    {
        McpTool t;
        t.name = "taskman_phase_add";
        t.description = "Add a new phase to the project.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Unique phase identifier"}};
        props["name"] = nlohmann::json{{"type", "string"}, {"description", "Phase name"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}, {"description", "Phase status"}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this phase. Lower values appear first. IMPORTANT: Always set this to control display order. Use increments of 10 (e.g. 10, 20, 30) to allow easy insertion later."}};
        t.inputSchema = make_schema(props, {"id", "name"});
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_phase_edit
    {
        McpTool t;
        t.name = "taskman_phase_edit";
        t.description = "Edit an existing phase.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Phase ID to edit"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}};
        props["sort-order"] = nlohmann::json{{"type", nlohmann::json::array({"string", "integer"})}, {"description", "Display order for this phase. Lower values appear first."}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools.push_back(t);
    }

    // taskman_phase_list
    {
        McpTool t;
        t.name = "taskman_phase_list";
        t.description = "List all phases in JSON format.";
        t.inputSchema = make_schema({});
        t.inputSchema["additionalProperties"] = false;
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_milestone_add
    {
        McpTool t;
        t.name = "taskman_milestone_add";
        t.description = "Add a new milestone to a phase.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Unique milestone identifier"}};
        props["phase"] = nlohmann::json{{"type", "string"}, {"description", "Parent phase ID"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["criterion"] = nlohmann::json{{"type", "string"}};
        props["reached"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"0", "1"})}, {"description", "0=not reached, 1=reached"}};
        t.inputSchema = make_schema(props, {"id", "phase"});
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_milestone_edit
    {
        McpTool t;
        t.name = "taskman_milestone_edit";
        t.description = "Edit an existing milestone.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Milestone ID to edit"}};
        props["name"] = nlohmann::json{{"type", "string"}};
        props["criterion"] = nlohmann::json{{"type", "string"}};
        props["reached"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"0", "1"})}};
        props["phase"] = nlohmann::json{{"type", "string"}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools.push_back(t);
    }

    // taskman_milestone_list
    {
        McpTool t;
        t.name = "taskman_milestone_list";
        t.description = "List milestones, optionally filtered by phase.";
        std::map<std::string, nlohmann::json> props;
        props["phase"] = nlohmann::json{{"type", "string"}, {"description", "Filter by phase ID"}};
        t.inputSchema = make_schema(props);
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_task_add
    {
        McpTool t;
        t.name = "taskman_task_add";
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
        tools.push_back(t);
    }

    // taskman_task_get
    {
        McpTool t;
        t.name = "taskman_task_get";
        t.description = "Get a task by ID in JSON or text format.";
        std::map<std::string, nlohmann::json> props;
        props["id"] = nlohmann::json{{"type", "string"}, {"description", "Task UUID"}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}};
        t.inputSchema = make_schema(props, {"id"});
        t.positional_keys = {"id"};
        tools.push_back(t);
    }

    // taskman_task_list
    {
        McpTool t;
        t.name = "taskman_task_list";
        t.description = "List tasks, optionally filtered by phase, status, or role.";
        std::map<std::string, nlohmann::json> props;
        props["phase"] = nlohmann::json{{"type", "string"}};
        props["status"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"to_do", "in_progress", "done"})}};
        props["role"] = nlohmann::json{{"type", "string"}, {"enum", get_roles_json_array()}};
        props["format"] = nlohmann::json{{"type", "string"}, {"enum", nlohmann::json::array({"json", "text"})}};
        t.inputSchema = make_schema(props);
        t.positional_keys = {};
        tools.push_back(t);
    }

    // taskman_task_edit
    {
        McpTool t;
        t.name = "taskman_task_edit";
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
        tools.push_back(t);
    }

    // taskman_task_dep_add
    {
        McpTool t;
        t.name = "taskman_task_dep_add";
        t.description = "Add a dependency between two tasks.";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that depends on another"}};
        props["dep-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that must be completed first"}};
        t.inputSchema = make_schema(props, {"task-id", "dep-id"});
        t.positional_keys = {"task-id", "dep-id"};
        tools.push_back(t);
    }

    // taskman_task_dep_remove
    {
        McpTool t;
        t.name = "taskman_task_dep_remove";
        t.description = "Remove a dependency between two tasks.";
        std::map<std::string, nlohmann::json> props;
        props["task-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that depends on another"}};
        props["dep-id"] = nlohmann::json{{"type", "string"}, {"description", "Task that must be completed first"}};
        t.inputSchema = make_schema(props, {"task-id", "dep-id"});
        t.positional_keys = {"task-id", "dep-id"};
        tools.push_back(t);
    }

    return tools;
}

void send_response(const nlohmann::json& response) {
    std::cout << response.dump() << "\n";
    std::cout.flush();
}

/** True si la ligne est vide ou uniquement des espaces. */
bool is_blank(const std::string& s) {
    for (char c : s) {
        if (c != ' ' && c != '\t' && c != '\r') return false;
    }
    return true;
}

/** Erreur JSON-RPC : -32700 Parse error (id = null). */
void send_parse_error() {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = nullptr;
    err["error"]["code"] = -32700;
    err["error"]["message"] = "Parse error";
    send_response(err);
}

/** Erreur -32600 Invalid Request. id = null si absent dans la requête. */
void send_invalid_request(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32600;
    err["error"]["message"] = "Invalid Request";
    send_response(err);
}

/** Erreur -32601 Method not found. */
void send_method_not_found(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32601;
    err["error"]["message"] = "Method not found";
    send_response(err);
}

/** Erreur -32602 Unsupported protocol version. */
void send_unsupported_protocol_version(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32602;
    err["error"]["message"] = "Unsupported protocol version";
    send_response(err);
}

/** Erreur -32602 Invalid params (avec message détaillé). */
void send_invalid_params(const nlohmann::json& id, const std::string& message) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32602;
    err["error"]["message"] = message;
    send_response(err);
}

/** Réponse tools/list. */
void send_tools_list(const nlohmann::json& id) {
    auto tools_list = get_mcp_tools();
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"]["tools"] = nlohmann::json::array();
    for (const auto& tool : tools_list) {
        nlohmann::json t;
        t["name"] = tool.name;
        t["description"] = tool.description;
        t["inputSchema"] = tool.inputSchema;
        resp["result"]["tools"].push_back(t);
    }
    send_response(resp);
}

/** Réponse ping. */
void send_ping_result(const nlohmann::json& id) {
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"] = nlohmann::json::object();
    send_response(resp);
}

/** Convertit une valeur JSON en string pour argv. */
std::string json_to_string(const nlohmann::json& val) {
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

/** Construit argv à partir des arguments JSON et de la config de l'outil. */
std::vector<std::string> build_argv(const McpTool& tool, const nlohmann::json& arguments) {
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

/** Capture stdout/stderr et appelle une fonction cmd_*. */
struct CaptureGuard {
    std::ostringstream oss_out, oss_err;
    std::streambuf* old_out, *old_err;

    CaptureGuard() {
        old_out = std::cout.rdbuf(oss_out.rdbuf());
        old_err = std::cerr.rdbuf(oss_err.rdbuf());
    }

    ~CaptureGuard() {
        std::cout.rdbuf(old_out);
        std::cerr.rdbuf(old_err);
    }

    std::string get_out() const { return oss_out.str(); }
    std::string get_err() const { return oss_err.str(); }
};

/** Obtient le chemin de la DB (identique à main.cpp). */
const char* get_db_path() {
    const char* env = std::getenv("TASKMAN_DB_NAME");
    return (env && env[0] != '\0') ? env : "project_tasks.db";
}

/** Dispatch name → cmd_* et exécution. */
int dispatch_tool_call(const std::string& name, const nlohmann::json& arguments,
                       std::string& output, bool& is_error) {
    auto tools_list = get_mcp_tools();
    const McpTool* tool = nullptr;
    for (const auto& t : tools_list) {
        if (t.name == name) {
            tool = &t;
            break;
        }
    }
    if (!tool) {
        return -1; // Outil inconnu
    }

    // Construire argv
    std::vector<std::string> argv_strs = build_argv(*tool, arguments);
    
    // Convertir en char*[] pour les cmd_*
    // cxxopts peut s'attendre à argv[0] (nom de la commande), donc on ajoute un élément factice
    std::vector<char*> argv_ptrs;
    std::string dummy_argv0 = "taskman";
    argv_ptrs.push_back(const_cast<char*>(dummy_argv0.c_str()));
    for (auto& s : argv_strs) {
        argv_ptrs.push_back(const_cast<char*>(s.c_str()));
    }

    // Ouvrir la DB
    Database db;
    if (!db.open(get_db_path())) {
        output = "Failed to open database";
        is_error = true;
        return 1;
    }

    // Capture stdout/stderr
    CaptureGuard guard;
    int exit_code = 0;

    try {
        // Dispatch vers la bonne cmd_*
        if (name == "taskman_init") {
            if (!taskman::init_schema(db)) {
                exit_code = 1;
            } else {
                output = "Database initialized.";
            }
        } else if (name == "taskman_phase_add") {
            exit_code = cmd_phase_add(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_phase_edit") {
            exit_code = cmd_phase_edit(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_phase_list") {
            exit_code = cmd_phase_list(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_milestone_add") {
            exit_code = cmd_milestone_add(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_milestone_edit") {
            exit_code = cmd_milestone_edit(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_milestone_list") {
            exit_code = cmd_milestone_list(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_add") {
            exit_code = cmd_task_add(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_get") {
            exit_code = cmd_task_get(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_list") {
            exit_code = cmd_task_list(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_edit") {
            exit_code = cmd_task_edit(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_dep_add") {
            exit_code = cmd_task_dep_add(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else if (name == "taskman_task_dep_remove") {
            exit_code = cmd_task_dep_remove(static_cast<int>(argv_ptrs.size()), argv_ptrs.data(), db);
        } else {
            output = "Unknown tool: " + name;
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

/** Réponse tools/call. */
void send_tools_call_result(const nlohmann::json& id, const std::string& name,
                            const nlohmann::json& arguments) {
    std::string output;
    bool is_error = false;
    int result = dispatch_tool_call(name, arguments, output, is_error);

    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;

    if (result == -1) {
        // Outil inconnu → erreur JSON-RPC
        send_invalid_params(id, "Unknown tool: " + name);
        return;
    }

    // Succès ou erreur métier → result avec content
    resp["result"]["content"] = nlohmann::json::array();
    nlohmann::json content_item;
    content_item["type"] = "text";
    content_item["text"] = output.empty() ? (is_error ? "Error" : "") : output;
    resp["result"]["content"].push_back(content_item);
    resp["result"]["isError"] = is_error;

    send_response(resp);
}

/** Réponse InitializeResult : protocolVersion, capabilities, serverInfo. */
void send_initialize_result(const nlohmann::json& id) {
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"]["protocolVersion"] = "2025-11-25";
    resp["result"]["capabilities"]["tools"]["listChanged"] = false;
    resp["result"]["serverInfo"]["name"] = "taskman";
    resp["result"]["serverInfo"]["version"] = TASKMAN_VERSION;
    send_response(resp);
}

/** True après réception de notifications/initialized (phase opération). */
bool g_operational = false;

} // namespace

int run_mcp_server() {
    std::string line;
    while (std::getline(std::cin, line)) {
        /* Trim trailing \r (Windows CRLF) pour que le JSON parse. */
        while (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (is_blank(line))
            continue;

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(line);
        } catch (const nlohmann::json::exception&) {
            send_parse_error();
            continue;
        }

        if (!j.is_object()) {
            send_invalid_request(nlohmann::json());
            continue;
        }
        if (!j.contains("jsonrpc") || j["jsonrpc"] != "2.0") {
            nlohmann::json id = j.contains("id") ? j["id"] : nlohmann::json();
            send_invalid_request(id);
            continue;
        }

        /* method (pour notifications et requêtes) */
        std::string method;
        if (j.contains("method") && j["method"].is_string())
            method = j["method"].get<std::string>();

        /* Notification : pas d'id → on ne répond pas. */
        if (!j.contains("id")) {
            if (method == "notifications/initialized")
                g_operational = true;
            continue;
        }

        nlohmann::json id = j["id"];

        /* initialize */
        if (method == "initialize") {
            nlohmann::json params = j.contains("params") ? j["params"] : nlohmann::json();
            if (!params.is_object() || !params.contains("protocolVersion")) {
                send_invalid_request(id);
                continue;
            }
            if (params["protocolVersion"] != "2025-11-25") {
                send_unsupported_protocol_version(id);
                continue;
            }
            send_initialize_result(id);
            continue;
        }

        /* tools/list */
        if (method == "tools/list") {
            send_tools_list(id);
            continue;
        }

        /* tools/call */
        if (method == "tools/call") {
            nlohmann::json params = j.contains("params") ? j["params"] : nlohmann::json();
            if (!params.is_object()) {
                send_invalid_params(id, "Invalid arguments");
                continue;
            }
            if (!params.contains("name") || !params["name"].is_string()) {
                send_invalid_params(id, "Invalid arguments: missing or invalid 'name'");
                continue;
            }
            // Vérifier que arguments est un objet s'il est présent
            if (params.contains("arguments") && !params["arguments"].is_object()) {
                send_invalid_params(id, "Invalid arguments: 'arguments' must be an object");
                continue;
            }
            std::string tool_name = params["name"].get<std::string>();
            nlohmann::json arguments = params.contains("arguments") && params["arguments"].is_object()
                ? params["arguments"] : nlohmann::json::object();
            send_tools_call_result(id, tool_name, arguments);
            continue;
        }

        /* ping */
        if (method == "ping") {
            send_ping_result(id);
            continue;
        }

        /* Autres méthodes : -32601 Method not found. */
        send_method_not_found(id);
    }
    return 0;
}

} // namespace taskman
