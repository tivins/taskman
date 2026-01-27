/**
 * Implémentation task:add, task:get, task:list, task:edit.
 */

#include "task.hpp"
#include "db.hpp"
#include "formats.hpp"
#include "roles.hpp"
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <uuid.h>
#include <random>
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

std::string generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

} // namespace

namespace taskman {

int cmd_task_add(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:add", "Add a task");
    opts.add_options()
        ("title", "Task title", cxxopts::value<std::string>())
        ("phase", "Phase ID", cxxopts::value<std::string>())
        ("description", "Description", cxxopts::value<std::string>())
        ("role", "Role", cxxopts::value<std::string>())
        ("milestone", "Milestone ID", cxxopts::value<std::string>())
        ("sort-order", "Sort order (integer)", cxxopts::value<std::string>())
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));

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

    if (!result.count("title")) {
        std::cerr << "taskman: --title is required\n";
        return 1;
    }
    if (!result.count("phase")) {
        std::cerr << "taskman: --phase is required\n";
        return 1;
    }

    std::string title, phase, description, role, milestone, sort_order_str, format;
    try {
        title = result["title"].as<std::string>();
        phase = result["phase"].as<std::string>();
        format = result["format"].as<std::string>();
        if (result.count("description")) description = result["description"].as<std::string>();
        if (result.count("role")) {
            role = result["role"].as<std::string>();
            if (!is_valid_role(role)) {
                std::cerr << get_roles_error_message();
                return 1;
            }
        }
        if (result.count("milestone")) milestone = result["milestone"].as<std::string>();
        if (result.count("sort-order")) sort_order_str = result["sort-order"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    if (format != "json" && format != "text") {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }
    if (!sort_order_str.empty()) {
        int n = 0;
        if (!parse_int(sort_order_str, n)) {
            std::cerr << "taskman: --sort-order must be an integer\n";
            return 1;
        }
    }

    std::string id = generate_uuid_v4();
    const char* sql = "INSERT INTO tasks (id, phase_id, milestone_id, title, description, status, sort_order, role) "
                      "VALUES (?, ?, ?, ?, ?, 'to_do', ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(phase);
    params.push_back(milestone.empty() ? std::nullopt : std::optional<std::string>(milestone));
    params.push_back(title);
    params.push_back(description.empty() ? std::nullopt : std::optional<std::string>(description));
    params.push_back(sort_order_str.empty() ? std::nullopt : std::optional<std::string>(sort_order_str));
    params.push_back(role.empty() ? std::nullopt : std::optional<std::string>(role));

    if (!db.run(sql, params)) return 1;

    auto rows = db.query("SELECT id, phase_id, milestone_id, title, description, status, sort_order, role FROM tasks WHERE id = ?",
                         {id});
    if (rows.empty()) {
        std::cerr << "taskman: failed to read created task\n";
        return 1;
    }
    if (format == "text") {
        print_task_text(rows[0]);
    } else {
        nlohmann::json obj;
        task_to_json(obj, rows[0]);
        std::cout << obj.dump() << "\n";
    }
    return 0;
}

int cmd_task_get(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:get", "Get a task by ID");
    opts.add_options()
        ("id", "Task ID", cxxopts::value<std::string>())
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));
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
        std::cerr << "taskman: task id is required\n";
        return 1;
    }

    std::string format = result["format"].as<std::string>();
    if (format != "json" && format != "text") {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto rows = db.query("SELECT id, phase_id, milestone_id, title, description, status, sort_order, role FROM tasks WHERE id = ?",
                         {id});
    if (rows.empty()) {
        std::cerr << "taskman: task not found: " << id << "\n";
        return 1;
    }
    if (format == "text") {
        print_task_text(rows[0]);
    } else {
        nlohmann::json obj;
        task_to_json(obj, rows[0]);
        std::cout << obj.dump() << "\n";
    }
    return 0;
}

int cmd_task_list(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:list", "List tasks");
    opts.add_options()
        ("phase", "Filter by phase ID", cxxopts::value<std::string>())
        ("status", "Filter by status", cxxopts::value<std::string>())
        ("role", "Filter by role", cxxopts::value<std::string>())
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));

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

    std::string format = result["format"].as<std::string>();
    if (format != "json" && format != "text") {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }
    if (result.count("status")) {
        if (!is_valid_status(result["status"].as<std::string>())) {
            std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
            return 1;
        }
    }
    if (result.count("role")) {
        if (!is_valid_role(result["role"].as<std::string>())) {
            std::cerr << get_roles_error_message();
            return 1;
        }
    }

    std::string sql = "SELECT id, phase_id, milestone_id, title, description, status, sort_order, role FROM tasks";
    std::vector<std::string> where_parts;
    std::vector<std::optional<std::string>> params;

    if (result.count("phase")) {
        where_parts.push_back("phase_id = ?");
        params.push_back(result["phase"].as<std::string>());
    }
    if (result.count("status")) {
        where_parts.push_back("status = ?");
        params.push_back(result["status"].as<std::string>());
    }
    if (result.count("role")) {
        where_parts.push_back("role = ?");
        params.push_back(result["role"].as<std::string>());
    }
    if (!where_parts.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_parts.size(); ++i) {
            if (i) sql += " AND ";
            sql += where_parts[i];
        }
    }
    sql += " ORDER BY phase_id, sort_order, id";

    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (params.empty()) {
        rows = db.query(sql.c_str());
    } else {
        rows = db.query(sql.c_str(), params);
    }

    if (format == "json") {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            task_to_json(obj, row);
            arr.push_back(obj);
        }
        std::cout << arr.dump() << "\n";
    } else {
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i) std::cout << "---\n";
            print_task_text(rows[i]);
        }
    }
    return 0;
}

int cmd_task_edit(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:edit", "Edit a task");
    opts.add_options()
        ("id", "Task ID", cxxopts::value<std::string>())
        ("title", "Title", cxxopts::value<std::string>())
        ("description", "Description", cxxopts::value<std::string>())
        ("status", "Status: to_do, in_progress, done", cxxopts::value<std::string>())
        ("role", "Role", cxxopts::value<std::string>())
        ("milestone", "Milestone ID", cxxopts::value<std::string>())
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
        std::cerr << "taskman: task id is required\n";
        return 1;
    }

    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (result.count("title")) {
        set_parts.push_back("title = ?");
        params.push_back(result["title"].as<std::string>());
    }
    if (result.count("description")) {
        set_parts.push_back("description = ?");
        params.push_back(result["description"].as<std::string>());
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
    if (result.count("role")) {
        std::string r = result["role"].as<std::string>();
        if (!is_valid_role(r)) {
            std::cerr << get_roles_error_message();
            return 1;
        }
        set_parts.push_back("role = ?");
        params.push_back(r);
    }
    if (result.count("milestone")) {
        set_parts.push_back("milestone_id = ?");
        params.push_back(result["milestone"].as<std::string>());
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
        return 0;
    }

    std::string sql = "UPDATE tasks SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    if (!db.run(sql.c_str(), params)) return 1;
    return 0;
}

int cmd_task_dep_add(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:dep:add", "Add a task dependency: task-id depends on dep-id");
    opts.add_options()
        ("task-id", "Task ID (the task that depends on another)", cxxopts::value<std::string>())
        ("dep-id", "Dependency task ID (must be completed first)", cxxopts::value<std::string>());
    opts.parse_positional({"task-id", "dep-id"});

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

    std::string task_id, dep_id;
    try {
        task_id = result["task-id"].as<std::string>();
        dep_id = result["dep-id"].as<std::string>();
    } catch (const cxxopts::exceptions::exception&) {
        std::cerr << "taskman: task:dep:add requires <task-id> and <dep-id>\n";
        return 1;
    }
    if (task_id.empty() || dep_id.empty()) {
        std::cerr << "taskman: task:dep:add requires <task-id> and <dep-id>\n";
        return 1;
    }
    if (task_id == dep_id) {
        std::cerr << "taskman: a task cannot depend on itself\n";
        return 1;
    }

    auto rows_task = db.query("SELECT 1 FROM tasks WHERE id = ?", {task_id});
    if (rows_task.empty()) {
        std::cerr << "taskman: task not found: " << task_id << "\n";
        return 1;
    }
    auto rows_dep = db.query("SELECT 1 FROM tasks WHERE id = ?", {dep_id});
    if (rows_dep.empty()) {
        std::cerr << "taskman: task not found: " << dep_id << "\n";
        return 1;
    }
    auto rows_exist = db.query("SELECT 1 FROM task_deps WHERE task_id = ? AND depends_on = ?",
                               {task_id, dep_id});
    if (!rows_exist.empty()) {
        std::cerr << "taskman: dependency already exists\n";
        return 1;
    }

    if (!db.run("INSERT INTO task_deps (task_id, depends_on) VALUES (?, ?)", {task_id, dep_id})) {
        return 1;
    }
    return 0;
}

int cmd_task_dep_remove(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:dep:remove", "Remove a task dependency");
    opts.add_options()
        ("task-id", "Task ID", cxxopts::value<std::string>())
        ("dep-id", "Dependency task ID to remove", cxxopts::value<std::string>());
    opts.parse_positional({"task-id", "dep-id"});

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

    std::string task_id, dep_id;
    try {
        task_id = result["task-id"].as<std::string>();
        dep_id = result["dep-id"].as<std::string>();
    } catch (const cxxopts::exceptions::exception&) {
        std::cerr << "taskman: task:dep:remove requires <task-id> and <dep-id>\n";
        return 1;
    }
    if (task_id.empty() || dep_id.empty()) {
        std::cerr << "taskman: task:dep:remove requires <task-id> and <dep-id>\n";
        return 1;
    }

    if (!db.run("DELETE FROM task_deps WHERE task_id = ? AND depends_on = ?", {task_id, dep_id})) {
        return 1;
    }
    return 0;
}

} // namespace taskman
