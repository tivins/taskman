/**
 * Implémentation taskman web : serveur HTTP (GET /, /style.css, /main.js, /task/:id, /tasks).
 * Inclure httplib.h en premier (avant Windows.h sur Windows).
 */
#include <httplib.h>
#include "web.hpp"
#include "db.hpp"
#include "formats.hpp"
#include "web_assets.generated.h"
#include <nlohmann/json.hpp>
#include <cxxopts.hpp>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

const char* const STATUS_VALUES[] = {"to_do", "in_progress", "done"};
const char* const ROLE_VALUES[] = {
    "project-manager", "project-designer", "software-architect",
    "developer", "summary-writer", "documentation-writer"};

bool is_valid_status(const std::string& s) {
    for (const char* v : STATUS_VALUES) {
        if (s == v) return true;
    }
    return false;
}

bool is_valid_role(const std::string& s) {
    for (const char* v : ROLE_VALUES) {
        if (s == v) return true;
    }
    return false;
}

const char HTML_PAGE[] = R"x(<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>taskman</title>
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <div id="app"></div>
  <script src="main.js" type="module"></script>
</body>
</html>
)x";

} // namespace

namespace taskman {

int cmd_web(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman web", "Start HTTP server for web UI");
    opts.add_options()
        ("host", "Bind address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
        ("port", "Port", cxxopts::value<std::string>()->default_value("8080"));

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << "\n\n"
                      << "Examples:\n"
                      << "  taskman web\n"
                      << "  taskman web --host 127.0.0.1 --port 8080\n\n";
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

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(HTML_PAGE, "text/html; charset=utf-8");
    });

    svr.Get("/style.css", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(STYLE_CSS, "text/css; charset=utf-8");
    });

    svr.Get("/main.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MAIN_JS, "application/javascript; charset=utf-8");
    });

    svr.Get("/task/:id", [&db](const httplib::Request& req, httplib::Response& res) {
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            res.status = 400;
            res.set_content(R"({"error":"missing id"})", "application/json");
            return;
        }
        std::string id = it->second;
        auto rows = db.query(
            "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role FROM tasks WHERE id = ?",
            {id});
        if (rows.empty()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }
        nlohmann::json obj;
        task_to_json(obj, rows[0]);
        res.set_content(obj.dump(), "application/json");
    });

    svr.Get("/tasks", [&db](const httplib::Request& req, httplib::Response& res) {
        int limit = 20, page = 1;
        if (req.has_param("limit")) {
            try {
                int v = std::stoi(req.get_param_value("limit"));
                if (v >= 1 && v <= 100) limit = v;
            } catch (...) {}
        }
        if (req.has_param("page")) {
            try {
                int v = std::stoi(req.get_param_value("page"));
                if (v >= 1) page = v;
            } catch (...) {}
        }
        int offset = (page - 1) * limit;

        std::string sql = "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role FROM tasks";
        std::vector<std::string> where_parts;
        std::vector<std::optional<std::string>> params;

        if (req.has_param("phase")) {
            where_parts.push_back("phase_id = ?");
            params.push_back(req.get_param_value("phase"));
        }
        if (req.has_param("status")) {
            std::string s = req.get_param_value("status");
            if (is_valid_status(s)) {
                where_parts.push_back("status = ?");
                params.push_back(s);
            }
        }
        if (req.has_param("role")) {
            std::string r = req.get_param_value("role");
            if (is_valid_role(r)) {
                where_parts.push_back("role = ?");
                params.push_back(r);
            }
        }
        if (!where_parts.empty()) {
            sql += " WHERE ";
            for (size_t i = 0; i < where_parts.size(); ++i) {
                if (i) sql += " AND ";
                sql += where_parts[i];
            }
        }
        sql += " ORDER BY phase_id, sort_order, id LIMIT ? OFFSET ?";
        params.push_back(std::to_string(limit));
        params.push_back(std::to_string(offset));

        std::vector<std::map<std::string, std::optional<std::string>>> rows = db.query(sql.c_str(), params);

        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            task_to_json(obj, row);
            arr.push_back(obj);
        }
        res.set_content(arr.dump(), "application/json");
    });

    std::cout << "taskman web: http://" << host << ":" << port << "/\n";
    if (!svr.listen(host.c_str(), port)) {
        std::cerr << "taskman: cannot bind " << host << ":" << port << "\n";
        return 1;
    }
    return 0;
}

} // namespace taskman
