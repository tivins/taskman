/**
 * Implémentation de MilestoneCommandParser.
 */

#include "milestone_command_parser.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <iostream>

namespace taskman {

bool MilestoneCommandParser::parse_int(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

bool MilestoneCommandParser::is_valid_reached(const std::string& s) {
    return s == "0" || s == "1";
}

int MilestoneCommandParser::parse_add(int argc, char* argv[]) {
    cxxopts::Options opts("taskman milestone:add", "Add a milestone");
    opts.add_options()
        ("id", "Milestone ID", cxxopts::value<std::string>())
        ("phase", "Phase ID", cxxopts::value<std::string>())
        ("name", "Milestone name", cxxopts::value<std::string>())
        ("criterion", "Criterion", cxxopts::value<std::string>())
        ("reached", "Reached: 0 or 1", cxxopts::value<std::string>()->default_value("0"))
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
    if (!result.count("phase")) {
        std::cerr << "taskman: --phase is required\n";
        return 1;
    }

    std::string id, phase, name, criterion, reached_str, format;
    try {
        id = result["id"].as<std::string>();
        phase = result["phase"].as<std::string>();
        reached_str = result["reached"].as<std::string>();
        format = result["format"].as<std::string>();
        if (result.count("name")) name = result["name"].as<std::string>();
        if (result.count("criterion")) criterion = result["criterion"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }
    if (!is_valid_reached(reached_str)) {
        std::cerr << "taskman: --reached must be 0 or 1\n";
        return 1;
    }
    if (!MilestoneFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }
    bool reached = (reached_str == "1");
    if (!service_.create_milestone(id, phase,
                                    name.empty() ? std::nullopt : std::optional<std::string>(name),
                                    criterion.empty() ? std::nullopt : std::optional<std::string>(criterion),
                                    reached)) return 1;

    auto milestone = service_.get_milestone(id);
    if (milestone.empty()) {
        std::cerr << "taskman: failed to read created milestone\n";
        return 1;
    }

    if (format == "text") {
        formatter_.format_text(milestone, std::cout);
    } else {
        formatter_.format_json(milestone, std::cout);
    }
    return 0;
}

int MilestoneCommandParser::parse_edit(int argc, char* argv[]) {
    cxxopts::Options opts("taskman milestone:edit", "Edit a milestone");
    opts.add_options()
        ("id", "Milestone ID", cxxopts::value<std::string>())
        ("name", "Milestone name", cxxopts::value<std::string>())
        ("criterion", "Criterion", cxxopts::value<std::string>())
        ("reached", "Reached: 0 or 1", cxxopts::value<std::string>())
        ("phase", "Phase ID", cxxopts::value<std::string>());
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
        std::cerr << "taskman: milestone id is required\n";
        return 1;
    }

    std::optional<std::string> name, criterion, phase_id;
    std::optional<bool> reached;

    if (result.count("name")) name = result["name"].as<std::string>();
    if (result.count("criterion")) criterion = result["criterion"].as<std::string>();
    if (result.count("reached")) {
        std::string r = result["reached"].as<std::string>();
        if (!is_valid_reached(r)) {
            std::cerr << "taskman: --reached must be 0 or 1\n";
            return 1;
        }
        reached = (r == "1");
    }
    if (result.count("phase")) phase_id = result["phase"].as<std::string>();

    if (!service_.update_milestone(id, name, criterion, reached, phase_id)) {
        return 1;
    }
    return 0;
}

int MilestoneCommandParser::parse_list(int argc, char* argv[]) {
    cxxopts::Options opts("taskman milestone:list", "List milestones");
    opts.add_options()
        ("phase", "Filter by phase ID", cxxopts::value<std::string>())
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
    if (!MilestoneFormatter::is_valid_format(format)) {
        std::cerr << "taskman: --format must be json or text\n";
        return 1;
    }

    std::optional<std::string> phase_id;
    if (result.count("phase")) phase_id = result["phase"].as<std::string>();

    auto milestones = service_.list_milestones(phase_id);

    if (format == "json") {
        formatter_.format_json_list(milestones, std::cout);
    } else {
        formatter_.format_text_list(milestones, std::cout);
    }
    return 0;
}

} // namespace taskman
