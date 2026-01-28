/**
 * Implémentation du gestionnaire de protocole JSON-RPC pour MCP.
 */

#include "mcp_protocol_handler.hpp"
#include "version.h"
#include <iostream>
#include <vector>

namespace taskman {

McpProtocolHandler::McpProtocolHandler() : operational_(false) {
}

void McpProtocolHandler::send_response(const nlohmann::json& response) {
    std::cout << response.dump() << "\n";
    std::cout.flush();
}

bool McpProtocolHandler::is_blank(const std::string& s) const {
    for (char c : s) {
        if (c != ' ' && c != '\t' && c != '\r') return false;
    }
    return true;
}

void McpProtocolHandler::send_parse_error() {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = nullptr;
    err["error"]["code"] = -32700;
    err["error"]["message"] = "Parse error";
    send_response(err);
}

void McpProtocolHandler::send_invalid_request(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32600;
    err["error"]["message"] = "Invalid Request";
    send_response(err);
}

void McpProtocolHandler::send_method_not_found(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32601;
    err["error"]["message"] = "Method not found";
    send_response(err);
}

void McpProtocolHandler::send_unsupported_protocol_version(const nlohmann::json& id) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32602;
    err["error"]["message"] = "Unsupported protocol version";
    send_response(err);
}

void McpProtocolHandler::send_invalid_params(const nlohmann::json& id, const std::string& message) {
    nlohmann::json err;
    err["jsonrpc"] = "2.0";
    err["id"] = id;
    err["error"]["code"] = -32602;
    err["error"]["message"] = message;
    send_response(err);
}

void McpProtocolHandler::send_initialize_result(const nlohmann::json& id) {
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"]["protocolVersion"] = "2025-11-25";
    resp["result"]["capabilities"]["tools"]["listChanged"] = false;
    resp["result"]["serverInfo"]["name"] = "taskman";
    resp["result"]["serverInfo"]["version"] = TASKMAN_VERSION;
    send_response(resp);
}

void McpProtocolHandler::send_ping_result(const nlohmann::json& id) {
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"] = nlohmann::json::object();
    send_response(resp);
}

void McpProtocolHandler::handle_initialize(const nlohmann::json& request, const nlohmann::json& id) {
    nlohmann::json params = request.contains("params") ? request["params"] : nlohmann::json();
    if (!params.is_object() || !params.contains("protocolVersion")) {
        send_invalid_request(id);
        return;
    }
    if (params["protocolVersion"] != "2025-11-25") {
        send_unsupported_protocol_version(id);
        return;
    }
    send_initialize_result(id);
}

void McpProtocolHandler::handle_initialized() {
    operational_ = true;
}

void McpProtocolHandler::handle_tools_list(const nlohmann::json& id, const std::vector<nlohmann::json>& tools_list) {
    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;
    resp["result"]["tools"] = nlohmann::json::array();
    for (const auto& tool : tools_list) {
        resp["result"]["tools"].push_back(tool);
    }
    send_response(resp);
}

void McpProtocolHandler::handle_tools_call(const nlohmann::json& id, const nlohmann::json& params,
                                          const std::string& output, bool is_error) {
    if (!params.is_object()) {
        send_invalid_params(id, "Invalid arguments");
        return;
    }
    if (!params.contains("name") || !params["name"].is_string()) {
        send_invalid_params(id, "Invalid arguments: missing or invalid 'name'");
        return;
    }
    // Vérifier que arguments est un objet s'il est présent
    if (params.contains("arguments") && !params["arguments"].is_object()) {
        send_invalid_params(id, "Invalid arguments: 'arguments' must be an object");
        return;
    }

    nlohmann::json resp;
    resp["jsonrpc"] = "2.0";
    resp["id"] = id;

    // Succès ou erreur métier → result avec content
    resp["result"]["content"] = nlohmann::json::array();
    nlohmann::json content_item;
    content_item["type"] = "text";
    content_item["text"] = output.empty() ? (is_error ? "Error" : "") : output;
    resp["result"]["content"].push_back(content_item);
    resp["result"]["isError"] = is_error;

    send_response(resp);
}

void McpProtocolHandler::send_tool_error(const nlohmann::json& id, const std::string& message) {
    send_invalid_params(id, message);
}

bool McpProtocolHandler::process_request(const std::string& line) {
    /* Trim trailing \r (Windows CRLF) pour que le JSON parse. */
    std::string trimmed_line = line;
    while (!trimmed_line.empty() && trimmed_line.back() == '\r')
        trimmed_line.pop_back();
    if (is_blank(trimmed_line))
        return true;

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(trimmed_line);
    } catch (const nlohmann::json::exception&) {
        send_parse_error();
        return true;
    }

    if (!j.is_object()) {
        send_invalid_request(nlohmann::json());
        return true;
    }
    if (!j.contains("jsonrpc") || j["jsonrpc"] != "2.0") {
        nlohmann::json id = j.contains("id") ? j["id"] : nlohmann::json();
        send_invalid_request(id);
        return true;
    }

    /* method (pour notifications et requêtes) */
    std::string method;
    if (j.contains("method") && j["method"].is_string())
        method = j["method"].get<std::string>();

    /* Notification : pas d'id → on ne répond pas. */
    if (!j.contains("id")) {
        if (method == "notifications/initialized")
            handle_initialized();
        return true;
    }

    nlohmann::json id = j["id"];

    /* initialize */
    if (method == "initialize") {
        handle_initialize(j, id);
        return true;
    }

    /* tools/list et tools/call sont gérés par le code appelant via les méthodes publiques */
    if (method == "tools/list" || method == "tools/call") {
        return false; // Indique que la méthode doit être gérée par le code appelant
    }

    /* ping */
    if (method == "ping") {
        send_ping_result(id);
        return true;
    }

    /* Autres méthodes : -32601 Method not found. */
    send_method_not_found(id);
    return true;
}

} // namespace taskman
