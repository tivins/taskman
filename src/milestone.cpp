/**
 * Implémentation milestone:add, milestone:edit, milestone:list.
 */

#include "milestone.hpp"
#include "db.hpp"
#include "formats.hpp"
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

bool parse_int(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

bool is_valid_reached(const std::string& s) {
    return s == "0" || s == "1";
}

} // namespace

namespace taskman {

bool milestone_add(Database& db,
                   const std::string& id,
                   const std::string& phase_id,
                   const std::optional<std::string>& name,
                   const std::optional<std::string>& criterion,
                   bool reached) {
    const char* sql = "INSERT INTO milestones (id, phase_id, name, criterion, reached) VALUES (?, ?, ?, ?, ?)";
    std::vector<std::optional<std::string>> params;
    params.push_back(id);
    params.push_back(phase_id);
    params.push_back(name);
    params.push_back(criterion);
    params.push_back(reached ? "1" : "0");
    return db.run(sql, params);
}

int cmd_milestone_add(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman milestone:add", "Add a milestone");
    opts.add_options()
        ("id", "Milestone ID", cxxopts::value<std::string>())
        ("phase", "Phase ID", cxxopts::value<std::string>())
        ("name", "Milestone name", cxxopts::value<std::string>())
        ("criterion", "Criterion", cxxopts::value<std::string>())
        ("reached", "Reached: 0 or 1", cxxopts::value<std::string>()->default_value("0"));

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

    std::string id, phase, name, criterion, reached_str;
    try {
        id = result["id"].as<std::string>();
        phase = result["phase"].as<std::string>();
        reached_str = result["reached"].as<std::string>();
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
    bool reached = (reached_str == "1");
    if (!milestone_add(db, id, phase,
                       name.empty() ? std::nullopt : std::optional<std::string>(name),
                       criterion.empty() ? std::nullopt : std::optional<std::string>(criterion),
                       reached)) return 1;
    return 0;
}

int cmd_milestone_edit(int argc, char* argv[], Database& db) {
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

    std::vector<std::string> set_parts;
    std::vector<std::optional<std::string>> params;

    if (result.count("name")) {
        set_parts.push_back("name = ?");
        params.push_back(result["name"].as<std::string>());
    }
    if (result.count("criterion")) {
        set_parts.push_back("criterion = ?");
        params.push_back(result["criterion"].as<std::string>());
    }
    if (result.count("reached")) {
        std::string r = result["reached"].as<std::string>();
        if (!is_valid_reached(r)) {
            std::cerr << "taskman: --reached must be 0 or 1\n";
            return 1;
        }
        set_parts.push_back("reached = ?");
        params.push_back(r);
    }
    if (result.count("phase")) {
        set_parts.push_back("phase_id = ?");
        params.push_back(result["phase"].as<std::string>());
    }

    if (set_parts.empty()) {
        return 0;
    }

    std::string sql = "UPDATE milestones SET ";
    for (size_t i = 0; i < set_parts.size(); ++i) {
        if (i) sql += ", ";
        sql += set_parts[i];
    }
    sql += " WHERE id = ?";
    params.push_back(id);

    if (!db.run(sql.c_str(), params)) return 1;
    return 0;
}

int cmd_milestone_list(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman milestone:list", "List milestones");
    opts.add_options()
        ("phase", "Filter by phase ID", cxxopts::value<std::string>());

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

    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (result.count("phase")) {
        std::string phase_id = result["phase"].as<std::string>();
        rows = db.query(
            "SELECT id, phase_id, name, criterion, reached FROM milestones WHERE phase_id = ? ORDER BY phase_id, id",
            {phase_id});
    } else {
        rows = db.query(
            "SELECT id, phase_id, name, criterion, reached FROM milestones ORDER BY phase_id, id");
    }

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& row : rows) {
        nlohmann::json obj;
        milestone_to_json(obj, row);
        arr.push_back(obj);
    }
    std::cout << arr.dump() << "\n";
    return 0;
}

} // namespace taskman
