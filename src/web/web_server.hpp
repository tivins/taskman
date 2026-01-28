/**
 * WebServer — infrastructure HTTP uniquement.
 * Responsabilité unique : gestion du serveur HTTP, routage de base, gestion des assets statiques.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_WEB_SERVER_HPP
#define TASKMAN_WEB_SERVER_HPP

#include <httplib.h>
#include <string>

namespace taskman {

class TaskController;
class PhaseController;
class MilestoneController;

/**
 * Serveur web qui gère uniquement l'infrastructure HTTP.
 * Les contrôleurs sont responsables de la logique métier.
 */
class WebServer {
public:
    /** Constructeur prenant les contrôleurs nécessaires. */
    WebServer(TaskController& task_controller,
              PhaseController& phase_controller,
              MilestoneController& milestone_controller);

    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

    /** Configure et démarre le serveur HTTP.
     * Retourne 0 en cas de succès, 1 en cas d'erreur. */
    int start(const std::string& host, int port, const std::string& assets_dir = "");

private:
    /** Enregistre les routes pour les assets statiques (HTML, CSS, JS). */
    void register_asset_routes(const std::string& assets_dir);

    /** Enregistre les routes des contrôleurs. */
    void register_controller_routes();

    httplib::Server svr_;
    TaskController& task_controller_;
    PhaseController& phase_controller_;
    MilestoneController& milestone_controller_;
};

} // namespace taskman

#endif /* TASKMAN_WEB_SERVER_HPP */
