/**
 * Implémentation de PhaseFormatter.
 */

#include "phase_formatter.hpp"
#include <iostream>

namespace taskman {

void PhaseFormatter::format_json(const std::map<std::string, std::optional<std::string>>& phase, std::ostream& out) {
    nlohmann::json obj;
    phase_to_json(obj, phase);
    out << obj.dump() << "\n";
}

void PhaseFormatter::format_text(const std::map<std::string, std::optional<std::string>>& phase, std::ostream& out) {
    auto get = [&phase](const char* k) -> std::string {
        auto it = phase.find(k);
        if (it == phase.end() || !it->second.has_value()) return "";
        return *it->second;
    };
    out << "id: " << get("id") << "\n";
    out << "name: " << get("name") << "\n";
    out << "status: " << get("status") << "\n";
    std::string so = get("sort_order");
    if (!so.empty()) out << "sort_order: " << so << "\n";
    std::string ca = get("created_at");
    if (!ca.empty()) out << "created_at: " << ca << "\n";
    std::string ua = get("updated_at");
    if (!ua.empty()) out << "updated_at: " << ua << "\n";
}

void PhaseFormatter::format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& phases, std::ostream& out) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& phase : phases) {
        nlohmann::json obj;
        phase_to_json(obj, phase);
        arr.push_back(obj);
    }
    out << arr.dump() << "\n";
}

void PhaseFormatter::format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& phases, std::ostream& out) {
    for (size_t i = 0; i < phases.size(); ++i) {
        if (i) out << "---\n";
        format_text(phases[i], out);
    }
}

bool PhaseFormatter::is_valid_format(const std::string& format) {
    return format == "json" || format == "text";
}

} // namespace taskman
