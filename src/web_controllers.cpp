/**
 * Implémentation des contrôleurs REST pour l'API web.
 */

#include "web_controllers.hpp"
#include "task_repository.hpp"
#include "task_service.hpp"
#include "task_formatter.hpp"
#include "phase_repository.hpp"
#include "milestone_repository.hpp"
#include "note_repository.hpp"
#include "formats.hpp"
#include "roles.hpp"
#include <nlohmann/json.hpp>
#include <climits>
#include <string>
#include <optional>

namespace taskman {

namespace {
    // Fonctions utilitaires pour parser les paramètres de requête
    int parse_int_param(const httplib::Request& req, const std::string& name, int default_value, int min_value, int max_value) {
        if (!req.has_param(name)) {
            return default_value;
        }
        try {
            int v = std::stoi(req.get_param_value(name));
            if (v >= min_value && v <= max_value) {
                return v;
            }
        } catch (...) {}
        return default_value;
    }

    std::optional<std::string> get_optional_param(const httplib::Request& req, const std::string& name) {
        if (!req.has_param(name)) {
            return std::nullopt;
        }
        return req.get_param_value(name);
    }
}

// TaskController

TaskController::TaskController(TaskRepository& task_repo, TaskService& task_service, NoteRepository& note_repo)
    : task_repo_(task_repo), task_service_(task_service), note_repo_(note_repo) {}

void TaskController::register_routes(httplib::Server& svr) {
    // GET /task/:id
    svr.Get("/task/:id", [this](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        auto task = task_repo_.get_by_id(id);
        if (task.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        nlohmann::json obj;
        task_to_json(obj, task);
        res.set_content(obj.dump(), "application/json");
    });

    // GET /task/:id/deps
    svr.Get("/task/:id/deps", [this](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        if (!task_repo_.exists(id)) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        auto rows = task_repo_.get_dependencies(id);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
            obj["task_id"] = get("task_id").value_or("");
            obj["depends_on"] = get("depends_on").value_or("");
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });

    // GET /task/:id/notes
    svr.Get("/task/:id/notes", [this](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        if (!task_repo_.exists(id)) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        auto rows = note_repo_.list_by_task_id(id);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
            obj["id"] = get("id").value_or("");
            obj["task_id"] = get("task_id").value_or("");
            obj["content"] = get("content").value_or("");
            obj["kind"] = get("kind").value_or("");
            obj["role"] = get("role").value_or("");
            obj["created_at"] = get("created_at").value_or("");
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });

    // GET /tasks/count
    svr.Get("/tasks/count", [this](const httplib::Request& req, httplib::Response& res) {
        std::optional<std::string> phase = get_optional_param(req, "phase");
        std::optional<std::string> milestone = get_optional_param(req, "milestone");
        std::optional<std::string> status = get_optional_param(req, "status");
        std::optional<std::string> role = get_optional_param(req, "role");

        // Valider le statut
        if (status.has_value() && !TaskService::is_valid_status(*status)) {
            status = std::nullopt;
        }

        // Valider le rôle
        if (role.has_value() && !is_valid_role(*role)) {
            role = std::nullopt;
        }

        int count = task_repo_.count(phase, milestone, status, role);
        nlohmann::json obj;
        obj["count"] = count;
        res.set_content(obj.dump(), "application/json");
    });

    // GET /tasks
    svr.Get("/tasks", [this](const httplib::Request& req, httplib::Response& res) {
        int limit = parse_int_param(req, "limit", 50, 1, 200);
        int page = parse_int_param(req, "page", 1, 1, INT_MAX);
        int offset = (page - 1) * limit;

        std::optional<std::string> phase = get_optional_param(req, "phase");
        std::optional<std::string> milestone = get_optional_param(req, "milestone");
        std::optional<std::string> status = get_optional_param(req, "status");
        std::optional<std::string> role = get_optional_param(req, "role");

        // Valider le statut
        if (status.has_value() && !TaskService::is_valid_status(*status)) {
            status = std::nullopt;
        }

        // Valider le rôle
        if (role.has_value() && !is_valid_role(*role)) {
            role = std::nullopt;
        }

        auto rows = task_repo_.list_paginated(phase, milestone, status, role, limit, offset);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            task_to_json(obj, row);
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });

    // GET /task_deps
    svr.Get("/task_deps", [this](const httplib::Request& req, httplib::Response& res) {
        int limit = parse_int_param(req, "limit", 100, 1, 500);
        int page = parse_int_param(req, "page", 1, 1, INT_MAX);
        int offset = (page - 1) * limit;

        std::optional<std::string> task_id = get_optional_param(req, "task_id");
        auto rows = task_repo_.list_dependencies(task_id, limit, offset);

        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
            obj["task_id"] = get("task_id").value_or("");
            obj["depends_on"] = get("depends_on").value_or("");
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });
}

// PhaseController

PhaseController::PhaseController(PhaseRepository& phase_repo) : phase_repo_(phase_repo) {}

void PhaseController::register_routes(httplib::Server& svr) {
    // GET /phase/:id
    svr.Get("/phase/:id", [this](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        auto phase = phase_repo_.get_by_id(id);
        if (phase.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        nlohmann::json obj;
        phase_to_json(obj, phase);
        res.set_content(obj.dump(), "application/json");
    });

    // GET /phases
    svr.Get("/phases", [this](const httplib::Request& req, httplib::Response& res) {
        int limit = parse_int_param(req, "limit", 30, 1, 100);
        int page = parse_int_param(req, "page", 1, 1, INT_MAX);
        int offset = (page - 1) * limit;

        auto rows = phase_repo_.list(limit, offset);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            phase_to_json(obj, row);
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });
}

// MilestoneController

MilestoneController::MilestoneController(MilestoneRepository& milestone_repo) : milestone_repo_(milestone_repo) {}

void MilestoneController::register_routes(httplib::Server& svr) {
    // GET /milestone/:id
    svr.Get("/milestone/:id", [this](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        auto milestone = milestone_repo_.get_by_id(id);
        if (milestone.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        nlohmann::json obj;
        milestone_to_json(obj, milestone);
        res.set_content(obj.dump(), "application/json");
    });

    // GET /milestones
    svr.Get("/milestones", [this](const httplib::Request& req, httplib::Response& res) {
        int limit = parse_int_param(req, "limit", 30, 1, 100);
        int page = parse_int_param(req, "page", 1, 1, INT_MAX);
        int offset = (page - 1) * limit;

        auto rows = milestone_repo_.list(limit, offset);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            milestone_to_json(obj, row);
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });
}

} // namespace taskman
