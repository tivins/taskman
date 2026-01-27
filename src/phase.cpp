/**
 * Implémentation phase:add, phase:edit, phase:list.
 */

#include "phase.hpp"
#include "db.hpp"
#include "formats.hpp"
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

const char* const STATUS_VALUES[] = {"to_do", "in_progress", "done"};

bool is_valid_status(const std::string& s) {
    for (const char* v : STATUS_VALUES) {
        if (s == v) return true;
    }
    return false;
}

bool parse_int(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

} // namespace

namespace taskman {

bool phase_add(Database& db,
               const std::string& id,
               const std::string& name,
               const std::string& status,
               std::optional<int> sort_order) {
    if (!is_valid_status(status)) {
        std::cerr << "taskman: invalid status\n";
        return false;
    }
    const char* sql = "INSERT INTO phases (id, name, status, sort_order) VALUES (?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(name);
    params.push_back(status);
    params.push_back(sort_order.has_value()
                     ? std::optional<std::string>(std::to_string(*sort_order))
                     : std::nullopt);
    return db.run(sql, params);
}

int cmd_phase_add(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman phase:add", "Add a phase");
    opts.add_options()
        ("id", "Phase ID", cxxopts::value<std::string>())
        ("name", "Phase name", cxxopts::value<std::string>())
        ("status", "Status: to_do, in_progress, done", cxxopts::value<std::string>()->default_value("to_do"))
        ("sort-order", "Sort order (integer)", cxxopts::value<std::string>());

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << '\n';
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

    if (!result.count("id")) {
        std::cerr << "taskman: --id is required\n";
        return 1;
    }
    if (!result.count("name")) {
        std::cerr << "taskman: --name is required\n";
        return 1;
    }

    std::string id, name, status, sort_order_str;
    try {
        id = result["id"].as<std::string>();
        name = result["name"].as<std::string>();
        status = result["status"].as<std::string>();
        if (result.count("sort-order")) sort_order_str = result["sort-order"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }
    if (!is_valid_status(status)) {
        std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
        return 1;
    }
    std::optional<int> sort_order_opt;
    if (!sort_order_str.empty()) {
        int n = 0;
        if (!parse_int(sort_order_str, n)) {
            std::cerr << "taskman: --sort-order must be an integer\n";
            return 1;
        }
        sort_order_opt = n;
    }
    if (!phase_add(db, id, name, status, sort_order_opt)) return 1;
    return 0;
}

int cmd_phase_edit(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman phase:edit", "Edit a phase");
    opts.add_options()
        ("id", "Phase ID", cxxopts::value<std::string>())
        ("name", "Phase name", cxxopts::value<std::string>())
        ("status", "Status: to_do, in_progress, done", cxxopts::value<std::string>())
        ("sort-order", "Sort order (integer)", cxxopts::value<std::string>());
    opts.parse_positional({"id"});

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << '\n';
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

    std::string id;
    try {
        id = result["id"].as<std::string>();
    } catch (const cxxopts::exceptions::exception&) {
        id.clear();
    }
    if (id.empty()) {
        std::cerr << "taskman: phase id is required\n";
        return 1;
    }

    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (result.count("name")) {
        set_parts.push_back("name = ?");
        params.push_back(result["name"].as<std::string>());
    }
    if (result.count("status")) {
        std::string s = result["status"].as<std::string>();
        if (!is_valid_status(s)) {
            std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
            return 1;
        }
        set_parts.push_back("status = ?");
        params.push_back(s);
    }
    if (result.count("sort-order")) {
        std::string so = result["sort-order"].as<std::string>();
        int n = 0;
        if (!parse_int(so, n)) {
            std::cerr << "taskman: --sort-order must be an integer\n";
            return 1;
        }
        set_parts.push_back("sort_order = ?");
        params.push_back(so);
    }

    if (set_parts.empty()) {
        return 0; // rien à mettre à jour
    }

    set_parts.push_back("updated_at = datetime('now')");

    std::string sql = "UPDATE phases SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    if (!db.run(sql.c_str(), params)) return 1;
    return 0;
}

int cmd_phase_list(int argc, char* argv[], Database& db) {
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "taskman phase:list\n\n"
                         "List all phases as a JSON array, ordered by sort_order.\nNo options.\n\n";
            return 0;
        }
    }
    auto rows = db.query("SELECT id, name, status, sort_order, created_at, updated_at FROM phases ORDER BY sort_order");
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& row : rows) {
        nlohmann::json obj;
        phase_to_json(obj, row);
        arr.push_back(obj);
    }
    std::cout << arr.dump() << "\n";
    return 0;
}

} // namespace taskman
