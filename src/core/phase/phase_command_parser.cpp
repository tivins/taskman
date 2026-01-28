/**
 * Implémentation de PhaseCommandParser.
 */

#include "phase_command_parser.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <iostream>

namespace taskman {

bool PhaseCommandParser::parse_int(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

int PhaseCommandParser::parse_add(int argc, char* argv[]) {
    cxxopts::Options opts("taskman phase:add", "Add a phase");
    opts.add_options()
        ("id", "Phase ID", cxxopts::value<std::string>())
        ("name", "Phase name", cxxopts::value<std::string>())
        ("status", "Status: to_do, in_progress, done", cxxopts::value<std::string>()->default_value("to_do"))
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

    if (!result.count("id")) {
        std::cerr << "taskman: --id is required\n";
        return 1;
    }
    if (!result.count("name")) {
        std::cerr << "taskman: --name is required\n";
        return 1;
    }

    std::string id, name, status, sort_order_str, format;
    try {
        id = result["id"].as<std::string>();
        name = result["name"].as<std::string>();
        status = result["status"].as<std::string>();
        format = result["format"].as<std::string>();
        if (result.count("sort-order")) sort_order_str = result["sort-order"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }
    if (!PhaseService::is_valid_status(status)) {
        std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
        return 1;
    }
    if (!PhaseFormatter::is_valid_format(format)) {
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
    if (!service_.create_phase(id, name, status, sort_order_opt)) return 1;

    auto phase = service_.get_phase(id);
    if (phase.empty()) {
        std::cerr << "taskman: failed to read created phase\n";
        return 1;
    }

    if (format == "text") {
        formatter_.format_text(phase, std::cout);
    } else {
        formatter_.format_json(phase, std::cout);
    }
    return 0;
}

int PhaseCommandParser::parse_edit(int argc, char* argv[]) {
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

    std::optional<std::string> name, status;
    std::optional<int> sort_order;

    if (result.count("name")) name = result["name"].as<std::string>();
    if (result.count("status")) {
        std::string s = result["status"].as<std::string>();
        if (!PhaseService::is_valid_status(s)) {
            std::cerr << "taskman: --status must be one of: to_do, in_progress, done\n";
            return 1;
        }
        status = s;
    }
    if (result.count("sort-order")) {
        std::string so = result["sort-order"].as<std::string>();
        int n = 0;
        if (!parse_int(so, n)) {
            std::cerr << "taskman: --sort-order must be an integer\n";
            return 1;
        }
        sort_order = n;
    }

    if (!service_.update_phase(id, name, status, sort_order)) {
        return 1;
    }
    return 0;
}

int PhaseCommandParser::parse_list(int argc, char* argv[]) {
    cxxopts::Options opts("taskman phase:list", "List phases");
    opts.add_options()
        ("format", "Output: json or text", cxxopts::value<std::string>()->default_value("json"));

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "taskman phase:list\n\n"
                         "List all phases as a JSON array, ordered by sort_order.\n"
                         "Options:\n"
                         "  --format json|text  Output format (default: json)\n\n";
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
    if (!PhaseFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    auto phases = service_.list_phases();

    if (format == "json") {
        formatter_.format_json_list(phases, std::cout);
    } else {
        formatter_.format_text_list(phases, std::cout);
    }
    return 0;
}

} // namespace taskman
