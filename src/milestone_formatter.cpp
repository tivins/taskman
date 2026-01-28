/**
 * Implémentation de MilestoneFormatter.
 */

#include "milestone_formatter.hpp"
#include <iostream>

namespace taskman {

void MilestoneFormatter::format_json(const std::map<std::string, std::optional<std::string>>& milestone, std::ostream& out) {
    nlohmann::json obj;
    milestone_to_json(obj, milestone);
    out << obj.dump() << "\n";
}

void MilestoneFormatter::format_text(const std::map<std::string, std::optional<std::string>>& milestone, std::ostream& out) {
    auto get = [&milestone](const char* k) -> std::string {
        auto it = milestone.find(k);
        if (it == milestone.end() || !it->second.has_value()) return "";
        return *it->second;
    };
    out << "id: " << get("id") << "\n";
    out << "phase_id: " << get("phase_id") << "\n";
    std::string name = get("name");
    if (!name.empty()) out << "name: " << name << "\n";
    std::string criterion = get("criterion");
    if (!criterion.empty()) out << "criterion: " << criterion << "\n";
    out << "reached: " << get("reached") << "\n";
    std::string ca = get("created_at");
    if (!ca.empty()) out << "created_at: " << ca << "\n";
    std::string ua = get("updated_at");
    if (!ua.empty()) out << "updated_at: " << ua << "\n";
}

void MilestoneFormatter::format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& milestones, std::ostream& out) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& milestone : milestones) {
        nlohmann::json obj;
        milestone_to_json(obj, milestone);
        arr.push_back(obj);
    }
    out << arr.dump() << "\n";
}

void MilestoneFormatter::format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& milestones, std::ostream& out) {
    for (size_t i = 0; i < milestones.size(); ++i) {
        if (i) out << "---\n";
        format_text(milestones[i], out);
    }
}

bool MilestoneFormatter::is_valid_format(const std::string& format) {
    return format == "json" || format == "text";
}

} // namespace taskman
