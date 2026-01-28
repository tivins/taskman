/**
 * Implémentation taskman web : serveur HTTP.
 * Utilise WebServer et les contrôleurs REST pour respecter le principe SRP.
 * Inclure httplib.h en premier (avant Windows.h sur Windows).
 */
#include <httplib.h>
#include "web.hpp"
#include "web_server.hpp"
#include "web_controllers.hpp"
#include "db.hpp"
#include "task_repository.hpp"
#include "task_service.hpp"
#include "phase_repository.hpp"
#include "milestone_repository.hpp"
#include "note_repository.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <string>

namespace taskman {

int cmd_web(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman web", "Start HTTP server for web UI");
    opts.add_options()
        ("host", "Bind address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
        ("port", "Port", cxxopts::value<std::string>()->default_value("8080"))
        ("serve-assets-from", "Serve CSS/JS from directory (dev); default: use embedded",
         cxxopts::value<std::string>()->default_value(""));

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << "\n\n"
                      << "Examples:\n"
                      << "  taskman web\n"
                      << "  taskman web --host 127.0.0.1 --port 8080\n"
                      << "  taskman web --serve-assets-from embed/web   # dev: edit CSS/JS and refresh\n\n";
            return 0;
        }
    }
    cxxopts::ParseResult result;
    try {
        result = opts.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    std::string host = result["host"].as<std::string>();
    int port = 8080;
    try {
        port = std::stoi(result["port"].as<std::string>());
    } catch (...) {}
    if (port <= 0 || port > 65535) {
        std::cerr << "taskman: --port must be 1..65535\n";
        return 1;
    }

    std::string assets_dir = result["serve-assets-from"].as<std::string>();

    // Créer les repositories et services
    QueryExecutor& executor = db.get_executor();
    TaskRepository task_repo(executor);
    TaskService task_service(task_repo);
    PhaseRepository phase_repo(executor);
    MilestoneRepository milestone_repo(executor);
    NoteRepository note_repo(executor);

    // Créer les contrôleurs
    TaskController task_controller(task_repo, task_service, note_repo);
    PhaseController phase_controller(phase_repo);
    MilestoneController milestone_controller(milestone_repo);

    // Créer et démarrer le serveur web
    WebServer server(task_controller, phase_controller, milestone_controller);
    return server.start(host, port, assets_dir);
}

} // namespace taskman
