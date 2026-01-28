/**
 * Gestionnaire du protocole JSON-RPC pour MCP.
 * Responsabilité unique : parsing, validation et construction de réponses JSON-RPC.
 * Respecte le principe SRP : séparation du protocole de la logique métier.
 */

#ifndef TASKMAN_MCP_PROTOCOL_HANDLER_HPP
#define TASKMAN_MCP_PROTOCOL_HANDLER_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace taskman {

/**
 * Gestionnaire du protocole JSON-RPC pour MCP.
 * Responsabilité : parsing, validation et réponses JSON-RPC uniquement.
 */
class McpProtocolHandler {
public:
    McpProtocolHandler();
    ~McpProtocolHandler() = default;

    /**
     * Traite une ligne JSON-RPC reçue sur stdin.
     * @param line Ligne JSON à traiter
     * @return true si le serveur doit continuer, false pour arrêter
     */
    bool process_request(const std::string& line);

    /**
     * Indique si le serveur est en phase opérationnelle (après initialized).
     */
    bool is_operational() const { return operational_; }

    /**
     * Traite une requête tools/list.
     * @param id ID de la requête
     * @param tools_list Liste des outils au format JSON
     */
    void handle_tools_list(const nlohmann::json& id, const std::vector<nlohmann::json>& tools_list);

    /**
     * Traite une requête tools/call.
     * @param id ID de la requête
     * @param params Paramètres de la requête
     * @param output Sortie de l'exécution de l'outil
     * @param is_error Indique si l'exécution a échoué
     */
    void handle_tools_call(const nlohmann::json& id, const nlohmann::json& params,
                          const std::string& output, bool is_error);

    /**
     * Envoie une erreur JSON-RPC pour un outil inconnu ou des paramètres invalides.
     * @param id ID de la requête
     * @param message Message d'erreur
     */
    void send_tool_error(const nlohmann::json& id, const std::string& message);

private:
    /**
     * Envoie une réponse JSON-RPC sur stdout.
     */
    void send_response(const nlohmann::json& response);

    /**
     * Vérifie si une ligne est vide ou uniquement des espaces.
     */
    bool is_blank(const std::string& s) const;

    /**
     * Erreur JSON-RPC : -32700 Parse error (id = null).
     */
    void send_parse_error();

    /**
     * Erreur -32600 Invalid Request.
     */
    void send_invalid_request(const nlohmann::json& id);

    /**
     * Erreur -32601 Method not found.
     */
    void send_method_not_found(const nlohmann::json& id);

    /**
     * Erreur -32602 Unsupported protocol version.
     */
    void send_unsupported_protocol_version(const nlohmann::json& id);

    /**
     * Erreur -32602 Invalid params (avec message détaillé).
     */
    void send_invalid_params(const nlohmann::json& id, const std::string& message);

    /**
     * Réponse initialize.
     */
    void send_initialize_result(const nlohmann::json& id);

    /**
     * Réponse ping.
     */
    void send_ping_result(const nlohmann::json& id);

    /**
     * Traite une requête initialize.
     */
    void handle_initialize(const nlohmann::json& request, const nlohmann::json& id);

    /**
     * Traite une notification initialized.
     */
    void handle_initialized();

    bool operational_;
};

} // namespace taskman

#endif /* TASKMAN_MCP_PROTOCOL_HANDLER_HPP */
