/**
 * MCP server — boucle stdio, parse JSON-RPC, dispatch (squelette + lifecycle).
 * Erreurs : -32700 Parse error, -32600 Invalid Request, -32601 Method not found,
 *           -32602 Unsupported protocol version.
 */

#include "mcp.hpp"
#include "version.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace taskman {

namespace {

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

        /* Autres méthodes : -32601 Method not found. */
        send_method_not_found(id);
    }
    return 0;
}

} // namespace taskman
