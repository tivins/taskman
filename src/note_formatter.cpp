/**
 * Implémentation de NoteFormatter.
 */

#include "note_formatter.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

namespace taskman {

void NoteFormatter::format_json(const std::map<std::string, std::optional<std::string>>& note, std::ostream& out) {
    auto get = [&note](const char* k) {
        return note.count(k) ? note.at(k) : std::optional<std::string>();
    };
    nlohmann::json obj;
    if (auto v = get("id"); v.has_value())
        obj["id"] = *v;
    else
        obj["id"] = nullptr;
    if (auto v = get("task_id"); v.has_value())
        obj["task_id"] = *v;
    else
        obj["task_id"] = nullptr;
    if (auto v = get("content"); v.has_value())
        obj["content"] = *v;
    else
        obj["content"] = nullptr;
    if (auto v = get("kind"); v.has_value() && !v->empty())
        obj["kind"] = *v;
    else
        obj["kind"] = nullptr;
    if (auto v = get("role"); v.has_value() && !v->empty())
        obj["role"] = *v;
    else
        obj["role"] = nullptr;
    if (auto v = get("created_at"); v.has_value())
        obj["created_at"] = *v;
    else
        obj["created_at"] = nullptr;
    out << obj.dump() << "\n";
}

void NoteFormatter::format_text(const std::map<std::string, std::optional<std::string>>& note, std::ostream& out) {
    auto get = [&note](const char* k) -> std::string {
        if (!note.count(k)) return "";
        const auto& v = note.at(k);
        return v.has_value() ? *v : "";
    };
    out << "id: " << get("id") << "\n";
    out << "task_id: " << get("task_id") << "\n";
    out << "content: " << get("content") << "\n";
    std::string kind = get("kind");
    if (!kind.empty())
        out << "kind: " << kind << "\n";
    std::string role = get("role");
    if (!role.empty())
        out << "role: " << role << "\n";
    out << "created_at: " << get("created_at") << "\n";
}

void NoteFormatter::format_json_list(const std::vector<std::map<std::string, std::optional<std::string>>>& notes, std::ostream& out) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& note : notes) {
        auto get = [&note](const char* k) {
            return note.count(k) ? note.at(k) : std::optional<std::string>();
        };
        nlohmann::json obj;
        if (auto v = get("id"); v.has_value())
            obj["id"] = *v;
        else
            obj["id"] = nullptr;
        if (auto v = get("task_id"); v.has_value())
            obj["task_id"] = *v;
        else
            obj["task_id"] = nullptr;
        if (auto v = get("content"); v.has_value())
            obj["content"] = *v;
        else
            obj["content"] = nullptr;
        if (auto v = get("kind"); v.has_value() && !v->empty())
            obj["kind"] = *v;
        else
            obj["kind"] = nullptr;
        if (auto v = get("role"); v.has_value() && !v->empty())
            obj["role"] = *v;
        else
            obj["role"] = nullptr;
        if (auto v = get("created_at"); v.has_value())
            obj["created_at"] = *v;
        else
            obj["created_at"] = nullptr;
        arr.push_back(obj);
    }
    out << arr.dump() << "\n";
}

void NoteFormatter::format_text_list(const std::vector<std::map<std::string, std::optional<std::string>>>& notes, std::ostream& out) {
    for (size_t i = 0; i < notes.size(); ++i) {
        if (i) out << "\n---\n\n";
        format_text(notes[i], out);
    }
}

bool NoteFormatter::is_valid_format(const std::string& format) {
    return format == "json" || format == "text";
}

} // namespace taskman
