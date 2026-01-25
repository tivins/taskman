/**
 * Formats de sortie (phase 7) : JSON et text.
 * — JSON : structures pour phase, milestone, task (champs optionnels = null si absents).
 * — Text : format lisible pour task (titre, description, status, role, puis id, phase_id, …).
 */

#ifndef TASKMAN_FORMATS_HPP
#define TASKMAN_FORMATS_HPP

#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace taskman {

using Row = std::map<std::string, std::optional<std::string>>;

/** Phase → JSON : id, name, status, sort_order (sort_order en int si entier). */
void phase_to_json(nlohmann::json& out, const Row& row);

/** Milestone → JSON : id, phase_id, name, criterion, reached (reached en int). */
void milestone_to_json(nlohmann::json& out, const Row& row);

/** Task → JSON : id, phase_id, milestone_id, title, description, status, sort_order, role. */
void task_to_json(nlohmann::json& out, const Row& row);

/** Task → format text lisible (titre, description, status, role, puis id, phase_id, …). */
void print_task_text(const Row& row);

} // namespace taskman

#endif /* TASKMAN_FORMATS_HPP */
