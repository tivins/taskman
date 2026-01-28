/**
 * Implémentation de TaskFormatter.
 */

#include "task_formatter.hpp"
#include <iostream>

namespace taskman {

void TaskFormatter::format_json(const std::map<std::string, std::optional<std::string>>& task, std::ostream& out) {
    nlohmann::json obj;
    task_to_json(obj, task);
    out << obj.dump() << "\n";
}

void TaskFormatter::format_text(const std::map<std::string, std::optional<std::string>>& task, std::ostream& out) {
    print_task_text(task);
}

void TaskFormatter::format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& tasks, std::ostream& out) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& task : tasks) {
        nlohmann::json obj;
        task_to_json(obj, task);
        arr.push_back(obj);
    }
    out << arr.dump() << "\n";
}

void TaskFormatter::format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& tasks, std::ostream& out) {
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (i) out << "---\n";
        print_task_text(tasks[i]);
    }
}

bool TaskFormatter::is_valid_format(const std::string& format) {
    return format == "json" || format == "text";
}

} // namespace taskman
