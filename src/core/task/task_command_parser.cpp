/**
 * Implémentation de TaskCommandParser.
 */

#include "task_command_parser.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <iostream>

namespace taskman {

bool TaskCommandParser::parse_int(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

int TaskCommandParser::parse_add(int argc, char* argv[]) {
    cxxopts::Options opts("taskman task:add", "Add a task");
    opts.add_options()
        ("title", "Task title", cxxopts::value<std::string>())
        ("phase", "Phase ID", cxxopts::value<std::string>())
        ("description", "Description", cxxopts::value<std::string>())
        ("role", "Role (assignee)", cxxopts::value<std::string>())
        ("creator", "Creator role (who created the task)", cxxopts::value<std::string>())
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

    std::string title, phase, description, role, creator, milestone, sort_order_str, format;
    try {
        title = result["title"].as<std::string>();
        phase = result["phase"].as<std::string>();
        format = result["format"].as<std::string>();
        if (result.count("description")) description = result["description"].as<std::string>();
        if (result.count("role")) role = result["role"].as<std::string>();
        if (result.count("creator")) creator = result["creator"].as<std::string>();
        if (result.count("milestone")) milestone = result["milestone"].as<std::string>();
        if (result.count("sort-order")) sort_order_str = result["sort-order"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    if (!TaskFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
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

    auto id = service_.create_task(
        phase,
        milestone.empty() ? std::nullopt : std::optional<std::string>(milestone),
        title,
        description.empty() ? std::nullopt : std::optional<std::string>(description),
        "to_do",
        sort_order_opt,
        role.empty() ? std::nullopt : std::optional<std::string>(role),
        creator.empty() ? std::nullopt : std::optional<std::string>(creator));
    if (!id.has_value()) {
        return 1;
    }

    auto task = service_.get_task(*id);
    if (task.empty()) {
        std::cerr << "taskman: failed to read created task\n";
        return 1;
    }

    if (format == "text") {
        formatter_.format_text(task, std::cout);
    } else {
        formatter_.format_json(task, std::cout);
    }
    return 0;
}

int TaskCommandParser::parse_get(int argc, char* argv[]) {
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
    if (!TaskFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto task = service_.get_task(id);
    if (task.empty()) {
        std::cerr << "taskman: task not found: " << id << "\n";
        return 1;
    }

    if (format == "text") {
        formatter_.format_text(task, std::cout);
    } else {
        formatter_.format_json(task, std::cout);
    }
    return 0;
}

int TaskCommandParser::parse_list(int argc, char* argv[]) {
    cxxopts::Options opts("taskman task:list", "List tasks");
    opts.add_options()
        ("phase", "Filter by phase ID", cxxopts::value<std::string>())
        ("status", "Filter by status", cxxopts::value<std::string>())
        ("role", "Filter by role", cxxopts::value<std::string>())
        ("blocked-filter", "Filter by blocked state: blocked (only blocked tasks) or unblocked (only non-blocked)", cxxopts::value<std::string>())
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
    if (!TaskFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }
    if (result.count("status")) {
        if (!TaskService::is_valid_status(result["status"].as<std::string>())) {
            std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
            return 1;
        }
    }
    if (result.count("blocked-filter")) {
        std::string bf = result["blocked-filter"].as<std::string>();
        if (bf != "blocked" && bf != "unblocked") {
            std::cerr << "taskman: --blocked-filter must be blocked or unblocked\n";
            return 1;
        }
    }

    std::optional<std::string> phase_id, status, role, blocked_filter;
    if (result.count("phase")) phase_id = result["phase"].as<std::string>();
    if (result.count("status")) status = result["status"].as<std::string>();
    if (result.count("role")) role = result["role"].as<std::string>();
    if (result.count("blocked-filter")) blocked_filter = result["blocked-filter"].as<std::string>();

    auto tasks = service_.list_tasks(phase_id, status, role, blocked_filter);

    if (format == "json") {
        formatter_.format_json_list(tasks, std::cout);
    } else {
        formatter_.format_text_list(tasks, std::cout);
    }
    return 0;
}

int TaskCommandParser::parse_edit(int argc, char* argv[]) {
    cxxopts::Options opts("taskman task:edit", "Edit a task");
    opts.add_options()
        ("id", "Task ID", cxxopts::value<std::string>())
        ("title", "Title", cxxopts::value<std::string>())
        ("description", "Description", cxxopts::value<std::string>())
        ("status", "Status: to_do, in_progress, done", cxxopts::value<std::string>())
        ("role", "Role (assignee)", cxxopts::value<std::string>())
        ("creator", "Creator role (who created the task)", cxxopts::value<std::string>())
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

    std::optional<std::string> title, description, status, role, creator, milestone;
    std::optional<int> sort_order;

    if (result.count("title")) title = result["title"].as<std::string>();
    if (result.count("description")) description = result["description"].as<std::string>();
    if (result.count("status")) status = result["status"].as<std::string>();
    if (result.count("role")) role = result["role"].as<std::string>();
    if (result.count("creator")) creator = result["creator"].as<std::string>();
    if (result.count("milestone")) milestone = result["milestone"].as<std::string>();
    if (result.count("sort-order")) {
        std::string so = result["sort-order"].as<std::string>();
        int n = 0;
        if (!parse_int(so, n)) {
            std::cerr << "taskman: --sort-order must be an integer\n";
            return 1;
        }
        sort_order = n;
    }

    if (!service_.update_task(id, title, description, status, role, milestone, sort_order, creator)) {
        return 1;
    }
    return 0;
}

int TaskCommandParser::parse_dep_add(int argc, char* argv[]) {
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
    if (!service_.add_task_dependency(task_id, dep_id)) {
        return 1;
    }
    return 0;
}

int TaskCommandParser::parse_dep_remove(int argc, char* argv[]) {
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

    if (!service_.remove_task_dependency(task_id, dep_id)) {
        return 1;
    }
    return 0;
}

} // namespace taskman
