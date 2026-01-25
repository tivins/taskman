/**
 * MCP server — boucle stdio, parse JSON-RPC, dispatch (squelette phase 1).
 * Erreurs : -32700 Parse error, -32600 Invalid Request, -32601 Method not found.
 */

#include "mcp.hpp"
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

} // namespace

int run_mcp_server() {
    std::string line;
    while (std::getline(std::cin, line)) {
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

        /* Notification : pas d'id → on ne répond pas. */
        if (!j.contains("id"))
            continue;

        nlohmann::json id = j["id"];
        /* Requête valide mais méthode non implémentée (squelette) : -32601. */
        send_method_not_found(id);
    }
    return 0;
}

} // namespace taskman
