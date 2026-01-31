/**
 * Implémentation de NoteCommandParser.
 */

#include "note_command_parser.hpp"
#include "util/roles.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <iostream>
#include <sstream>

namespace taskman {

int NoteCommandParser::parse_add(int argc, char* argv[]) {
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
    if (!NoteFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto id = service_.create_note(task_id, content, kind, role);
    if (!id.has_value()) {
        return 1;
    }

    auto note = service_.get_note(*id);
    if (note.empty()) {
        std::cerr << "taskman: failed to read created note\n";
        return 1;
    }
    if (format == "text") {
        formatter_.format_text(note, std::cout);
    } else {
        formatter_.format_json(note, std::cout);
    }
    return 0;
}

int NoteCommandParser::parse_list(int argc, char* argv[]) {
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
    if (!NoteFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto notes = service_.list_notes(task_id);

    if (format == "text") {
        formatter_.format_text_list(notes, std::cout);
    } else {
        formatter_.format_json_list(notes, std::cout);
    }
    return 0;
}

int NoteCommandParser::parse_list_by_ids(int argc, char* argv[]) {
    cxxopts::Options opts("taskman task:note:list-by-ids", "List notes by a comma-separated list of note IDs");
    opts.add_options()
        ("ids", "Comma-separated note IDs", cxxopts::value<std::string>())
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
    if (!result.count("ids")) {
        std::cerr << "taskman: task:note:list-by-ids requires --ids <id1,id2,...>\n";
        return 1;
    }
    std::string ids_str = result["ids"].as<std::string>();
    std::vector<std::string> ids;
    std::stringstream ss(ids_str);
    std::string id;
    while (std::getline(ss, id, ',')) {
        // trim leading/trailing spaces
        auto start = id.find_first_not_of(" \t");
        if (start != std::string::npos) {
            auto end = id.find_last_not_of(" \t");
            ids.push_back(id.substr(start, end - start + 1));
        } else if (!id.empty()) {
            ids.push_back(id);
        }
    }
    std::string format = result["format"].as<std::string>();
    if (!NoteFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }
    auto notes = service_.list_notes_by_ids(ids);
    if (format == "text") {
        formatter_.format_text_list(notes, std::cout);
    } else {
        formatter_.format_json_list(notes, std::cout);
    }
    return 0;
}

} // namespace taskman
