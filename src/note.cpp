/**
 * Implémentation task:note:add, task:note:list.
 */

#include "note.hpp"
#include "db.hpp"
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

std::string generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

void note_to_json(nlohmann::json& out, const std::map<std::string, std::optional<std::string>>& row) {
    auto get = [&row](const char* k) {
        return row.count(k) ? row.at(k) : std::optional<std::string>();
    };
    if (auto v = get("id"); v.has_value())
        out["id"] = *v;
    else
        out["id"] = nullptr;
    if (auto v = get("task_id"); v.has_value())
        out["task_id"] = *v;
    else
        out["task_id"] = nullptr;
    if (auto v = get("content"); v.has_value())
        out["content"] = *v;
    else
        out["content"] = nullptr;
    if (auto v = get("kind"); v.has_value() && !v->empty())
        out["kind"] = *v;
    else
        out["kind"] = nullptr;
    if (auto v = get("role"); v.has_value() && !v->empty())
        out["role"] = *v;
    else
        out["role"] = nullptr;
    if (auto v = get("created_at"); v.has_value())
        out["created_at"] = *v;
    else
        out["created_at"] = nullptr;
}

void print_note_text(const std::map<std::string, std::optional<std::string>>& row) {
    auto get = [&row](const char* k) -> std::string {
        if (!row.count(k)) return "";
        const auto& v = row.at(k);
        return v.has_value() ? *v : "";
    };
    std::cout << "id: " << get("id") << "\n";
    std::cout << "task_id: " << get("task_id") << "\n";
    std::cout << "content: " << get("content") << "\n";
    if (!get("kind").empty())
        std::cout << "kind: " << get("kind") << "\n";
    if (!get("role").empty())
        std::cout << "role: " << get("role") << "\n";
    std::cout << "created_at: " << get("created_at") << "\n";
}

} // namespace

namespace taskman {

bool note_add(Database& db,
              const std::string& id,
              const std::string& task_id,
              const std::string& content,
              const std::optional<std::string>& kind,
              const std::optional<std::string>& role) {
    auto rows_task = db.query("SELECT 1 FROM tasks WHERE id = ?", {task_id});
    if (rows_task.empty()) {
        std::cerr << "taskman: task not found: " << task_id << "\n";
        return false;
    }
    if (role.has_value() && !is_valid_role(*role)) {
        std::cerr << get_roles_error_message();
        return false;
    }
    const char* sql = "INSERT INTO task_notes (id, task_id, content, kind, role) VALUES (?, ?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params = {id, task_id, content, kind, role};
    if (!db.run(sql, params)) return false;
    // Update task.updated_at when a note is added
    return db.run("UPDATE tasks SET updated_at = datetime('now') WHERE id = ?", {task_id});
}

int cmd_note_add(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:note:add", "Add a note to a task");
    opts.add_options()
        ("task-id", "Task ID", cxxopts::value<std::string>())
        ("content", "Note content", cxxopts::value<std::string>())
        ("kind", "Note kind: completion, progress, issue", cxxopts::value<std::string>())
        ("role", "Role of the agent who added the note", cxxopts::value<std::string>())
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));
    opts.parse_positional({"task-id"});

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

    std::string task_id;
    try {
        task_id = result["task-id"].as<std::string>();
    } catch (const cxxopts::exceptions::exception&) {
        task_id.clear();
    }
    if (task_id.empty()) {
        std::cerr << "taskman: task:note:add requires <task-id>\n";
        return 1;
    }
    if (!result.count("content")) {
        std::cerr << "taskman: --content is required\n";
        return 1;
    }
    std::string content = result["content"].as<std::string>();

    std::optional<std::string> kind, role;
    if (result.count("kind"))
        kind = result["kind"].as<std::string>();
    if (result.count("role")) {
        std::string r = result["role"].as<std::string>();
        if (!is_valid_role(r)) {
            std::cerr << get_roles_error_message();
            return 1;
        }
        role = r;
    }

    std::string format = result["format"].as<std::string>();
    if (format != "json" && format != "text") {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    std::string id = generate_uuid_v4();
    if (!note_add(db, id, task_id, content, kind, role))
        return 1;

    auto rows = db.query(
        "SELECT id, task_id, content, kind, role, created_at FROM task_notes WHERE id = ?",
        {id});
    if (rows.empty()) {
        std::cerr << "taskman: failed to read created note\n";
        return 1;
    }
    if (format == "text") {
        print_note_text(rows[0]);
    } else {
        nlohmann::json obj;
        note_to_json(obj, rows[0]);
        std::cout << obj.dump() << "\n";
    }
    return 0;
}

int cmd_note_list(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman task:note:list", "List notes for a task");
    opts.add_options()
        ("task-id", "Task ID", cxxopts::value<std::string>())
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));
    opts.parse_positional({"task-id"});

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

    std::string task_id;
    try {
        task_id = result["task-id"].as<std::string>();
    } catch (const cxxopts::exceptions::exception&) {
        task_id.clear();
    }
    if (task_id.empty()) {
        std::cerr << "taskman: task:note:list requires <task-id>\n";
        return 1;
    }

    std::string format = result["format"].as<std::string>();
    if (format != "json" && format != "text") {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto rows = db.query(
        "SELECT id, task_id, content, kind, role, created_at FROM task_notes WHERE task_id = ? ORDER BY created_at",
        {task_id});

    if (format == "text") {
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i) std::cout << "\n---\n\n";
            print_note_text(rows[i]);
        }
    } else {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json obj;
            note_to_json(obj, row);
            arr.push_back(obj);
        }
        std::cout << arr.dump() << "\n";
    }
    return 0;
}

} // namespace taskman
