/**
 * Implémentation des formats de sortie : JSON (phase, milestone, task) et text (task).
 */

#include "formats.hpp"
#include <iostream>
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

void set_or_null(nlohmann::json& obj, const std::string& key,
                 const std::optional<std::string>& v) {
    if (!v.has_value()) {
        obj[key] = nullptr;
        return;
    }
    obj[key] = *v;
}

void set_or_null_int(nlohmann::json& obj, const std::string& key,
                     const std::optional<std::string>& v) {
    if (!v.has_value()) {
        obj[key] = nullptr;
        return;
    }
    int n = 0;
    if (parse_int(*v, n)) {
        obj[key] = n;
    } else {
        obj[key] = *v;
    }
}

} // namespace

namespace taskman {

void phase_to_json(nlohmann::json& out, const Row& row) {
    auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
    set_or_null(out, "id", get("id"));
    set_or_null(out, "name", get("name"));
    set_or_null(out, "status", get("status"));
    set_or_null_int(out, "sort_order", get("sort_order"));
    set_or_null(out, "created_at", get("created_at"));
    set_or_null(out, "updated_at", get("updated_at"));
}

void milestone_to_json(nlohmann::json& out, const Row& row) {
    auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
    set_or_null(out, "id", get("id"));
    set_or_null(out, "phase_id", get("phase_id"));
    set_or_null(out, "name", get("name"));
    set_or_null(out, "criterion", get("criterion"));
    set_or_null_int(out, "reached", get("reached"));
    set_or_null(out, "created_at", get("created_at"));
    set_or_null(out, "updated_at", get("updated_at"));
}

namespace {

std::vector<std::string> split_note_ids(const std::string& s) {
    std::vector<std::string> ids;
    if (s.empty()) return ids;
    std::string acc;
    for (char c : s) {
        if (c == ',') {
            if (!acc.empty()) ids.push_back(acc);
            acc.clear();
        } else {
            acc += c;
        }
    }
    if (!acc.empty()) ids.push_back(acc);
    return ids;
}

} // namespace

void task_to_json(nlohmann::json& out, const Row& row) {
    auto get = [&row](const char* k) { return row.count(k) ? row.at(k) : std::nullopt; };
    set_or_null(out, "id", get("id"));
    set_or_null(out, "phase_id", get("phase_id"));
    set_or_null(out, "milestone_id", get("milestone_id"));
    set_or_null(out, "title", get("title"));
    set_or_null(out, "description", get("description"));
    set_or_null(out, "status", get("status"));
    set_or_null_int(out, "sort_order", get("sort_order"));
    set_or_null(out, "role", get("role"));
    set_or_null(out, "creator", get("creator"));
    set_or_null(out, "created_at", get("created_at"));
    set_or_null(out, "updated_at", get("updated_at"));
    // note_ids : liste des UID des notes liées (stockée en comma-separated dans la row)
    nlohmann::json arr = nlohmann::json::array();
    auto note_ids_opt = get("note_ids");
    if (note_ids_opt.has_value() && !note_ids_opt->empty()) {
        for (const auto& id : split_note_ids(*note_ids_opt)) {
            arr.push_back(id);
        }
    }
    out["note_ids"] = std::move(arr);
}

void print_task_text(const Row& row) {
    auto get = [&row](const char* k) -> std::string {
        auto it = row.find(k);
        if (it == row.end() || !it->second.has_value()) return "";
        return *it->second;
    };
    // Spec : titre, description, status, role sur lignes distinctes ; puis id, phase_id, … ; created_at, updated_at ; note_ids
    std::cout << "title: " << get("title") << "\n";
    std::cout << "description: " << get("description") << "\n";
    std::cout << "status: " << get("status") << "\n";
    std::cout << "role: " << get("role") << "\n";
    std::cout << "creator: " << get("creator") << "\n";
    std::cout << "id: " << get("id") << "\n";
    std::cout << "phase_id: " << get("phase_id") << "\n";
    std::cout << "milestone_id: " << get("milestone_id") << "\n";
    std::string so = get("sort_order");
    if (!so.empty()) std::cout << "sort_order: " << so << "\n";
    std::string ca = get("created_at");
    if (!ca.empty()) std::cout << "created_at: " << ca << "\n";
    std::string ua = get("updated_at");
    if (!ua.empty()) std::cout << "updated_at: " << ua << "\n";
    std::string note_ids = get("note_ids");
    if (!note_ids.empty()) {
        std::cout << "note_ids: " << note_ids << "\n";
    }
}

} // namespace taskman
