/**
 * Implémentation de WebServer.
 */

#include "web_server.hpp"
#include "web_controllers.hpp"
#include "web_assets.generated.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace taskman {

namespace {
    const char HTML_PAGE[] = R"x(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>taskman</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Mono&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <div class="box">
    <h1><span class="task-">task</span><span class="-man">man</span></h1>
    <div id="app"></div>
  </div>
  <script src="filters.js" type="module"></script>
  <script src="pagination.js" type="module"></script>
  <script src="main.js" type="module"></script>
</body>
</html>
)x";

    /** Read file into string; returns false on error. */
    bool read_file_to_string(const std::string& path, std::string& out) {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;
        out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        return true;
    }

    /** Build path base/name, normalizing to a single slash. */
    std::string asset_path(const std::string& base, const std::string& name) {
        if (base.empty()) return "";
        std::string p = base;
        while (!p.empty() && (p.back() == '/' || p.back() == '\\')) p.pop_back();
        return p + "/" + name;
    }
}

WebServer::WebServer(TaskController& task_controller,
                     PhaseController& phase_controller,
                     MilestoneController& milestone_controller)
    : task_controller_(task_controller),
      phase_controller_(phase_controller),
      milestone_controller_(milestone_controller) {}

void WebServer::register_asset_routes(const std::string& assets_dir) {
    // GET /
    svr_.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(HTML_PAGE, "text/html; charset=utf-8");
    });

    // GET /style.css
    svr_.Get("/style.css", [assets_dir](const httplib::Request&, httplib::Response& res) {
        if (!assets_dir.empty()) {
            std::string content;
            if (read_file_to_string(asset_path(assets_dir, "style.css"), content)) {
                res.set_header("Cache-Control", "no-store");
                res.set_content(content, "text/css; charset=utf-8");
                return;
            }
        }
        res.set_content(STYLE_CSS, "text/css; charset=utf-8");
    });

    // GET /dom.js
    svr_.Get("/dom.js", [assets_dir](const httplib::Request&, httplib::Response& res) {
        if (!assets_dir.empty()) {
            std::string content;
            if (read_file_to_string(asset_path(assets_dir, "dom.js"), content)) {
                res.set_header("Cache-Control", "no-store");
                res.set_content(content, "application/javascript; charset=utf-8");
                return;
            }
        }
        res.set_content(DOM_JS, "application/javascript; charset=utf-8");
    });

    // GET /filters.js
    svr_.Get("/filters.js", [assets_dir](const httplib::Request&, httplib::Response& res) {
        if (!assets_dir.empty()) {
            std::string content;
            if (read_file_to_string(asset_path(assets_dir, "filters.js"), content)) {
                res.set_header("Cache-Control", "no-store");
                res.set_content(content, "application/javascript; charset=utf-8");
                return;
            }
        }
        res.set_content(FILTERS_JS, "application/javascript; charset=utf-8");
    });

    // GET /pagination.js
    svr_.Get("/pagination.js", [assets_dir](const httplib::Request&, httplib::Response& res) {
        if (!assets_dir.empty()) {
            std::string content;
            if (read_file_to_string(asset_path(assets_dir, "pagination.js"), content)) {
                res.set_header("Cache-Control", "no-store");
                res.set_content(content, "application/javascript; charset=utf-8");
                return;
            }
        }
        res.set_content(PAGINATION_JS, "application/javascript; charset=utf-8");
    });

    // GET /main.js
    svr_.Get("/main.js", [assets_dir](const httplib::Request&, httplib::Response& res) {
        if (!assets_dir.empty()) {
            std::string content;
            if (read_file_to_string(asset_path(assets_dir, "main.js"), content)) {
                res.set_header("Cache-Control", "no-store");
                res.set_content(content, "application/javascript; charset=utf-8");
                return;
            }
        }
        res.set_content(MAIN_JS, "application/javascript; charset=utf-8");
    });
}

void WebServer::register_controller_routes() {
    task_controller_.register_routes(svr_);
    phase_controller_.register_routes(svr_);
    milestone_controller_.register_routes(svr_);
}

int WebServer::start(const std::string& host, int port, const std::string& assets_dir) {
    register_asset_routes(assets_dir);
    register_controller_routes();

    std::cout << "taskman web: http://" << host << ":" << port << "/\n";
    if (!svr_.listen(host.c_str(), port)) {
        std::cerr << "taskman: cannot bind " << host << ":" << port << "\n";
        return 1;
    }
    return 0;
}

} // namespace taskman
