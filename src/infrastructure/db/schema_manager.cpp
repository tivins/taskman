/**
 * Implémentation de SchemaManager.
 */

#include "schema_manager.hpp"
#include <string>

namespace taskman {

bool SchemaManager::table_has_column(const char* table, const char* column) {
    std::string sql = "SELECT name FROM pragma_table_info('";
    sql += table;
    sql += "') WHERE name = ?";
    auto rows = executor_.query(sql.c_str(), {std::string(column)});
    return !rows.empty();
}

bool SchemaManager::ensure_timestamps(const char* table) {
    if (!table_has_column(table, "created_at")) {
        std::string sql = "ALTER TABLE ";
        sql += table;
        sql += " ADD COLUMN created_at TEXT DEFAULT (datetime('now'))";
        if (!executor_.exec(sql.c_str())) return false;
    }
    if (!table_has_column(table, "updated_at")) {
        std::string sql = "ALTER TABLE ";
        sql += table;
        sql += " ADD COLUMN updated_at TEXT DEFAULT (datetime('now'))";
        if (!executor_.exec(sql.c_str())) return false;
    }
    return true;
}

bool SchemaManager::init_schema() {
    static const char* const phases_sql =
        "CREATE TABLE IF NOT EXISTS phases (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  name TEXT NOT NULL,\n"
        "  status TEXT DEFAULT 'to_do',\n"
        "  sort_order INTEGER,\n"
        "  created_at TEXT DEFAULT (datetime('now')),\n"
        "  updated_at TEXT DEFAULT (datetime('now'))\n"
        ");";
    if (!executor_.exec(phases_sql)) return false;

    static const char* const milestones_sql =
        "CREATE TABLE IF NOT EXISTS milestones (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  phase_id TEXT NOT NULL,\n"
        "  name TEXT,\n"
        "  criterion TEXT,\n"
        "  reached INTEGER DEFAULT 0,\n"
        "  created_at TEXT DEFAULT (datetime('now')),\n"
        "  updated_at TEXT DEFAULT (datetime('now')),\n"
        "  FOREIGN KEY (phase_id) REFERENCES phases(id)\n"
        ");";
    if (!executor_.exec(milestones_sql)) return false;

    static const char* const tasks_sql =
        "CREATE TABLE IF NOT EXISTS tasks (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  phase_id TEXT NOT NULL,\n"
        "  milestone_id TEXT,\n"
        "  title TEXT,\n"
        "  description TEXT,\n"
        "  status TEXT DEFAULT 'to_do',\n"
        "  sort_order INTEGER,\n"
        "  role TEXT,\n"
        "  created_at TEXT DEFAULT (datetime('now')),\n"
        "  updated_at TEXT DEFAULT (datetime('now')),\n"
        "  FOREIGN KEY (phase_id) REFERENCES phases(id),\n"
        "  FOREIGN KEY (milestone_id) REFERENCES milestones(id)\n"
        ");";
    if (!executor_.exec(tasks_sql)) return false;

    static const char* const task_deps_sql =
        "CREATE TABLE IF NOT EXISTS task_deps (\n"
        "  task_id TEXT NOT NULL,\n"
        "  depends_on TEXT NOT NULL,\n"
        "  PRIMARY KEY (task_id, depends_on),\n"
        "  FOREIGN KEY (task_id) REFERENCES tasks(id),\n"
        "  FOREIGN KEY (depends_on) REFERENCES tasks(id)\n"
        ");";
    if (!executor_.exec(task_deps_sql)) return false;

    static const char* const task_notes_sql =
        "CREATE TABLE IF NOT EXISTS task_notes (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  task_id TEXT NOT NULL,\n"
        "  content TEXT NOT NULL,\n"
        "  kind TEXT,\n"
        "  role TEXT,\n"
        "  created_at TEXT DEFAULT (datetime('now')),\n"
        "  FOREIGN KEY (task_id) REFERENCES tasks(id)\n"
        ");";
    if (!executor_.exec(task_notes_sql)) return false;

    if (!ensure_timestamps("phases")) return false;
    if (!ensure_timestamps("milestones")) return false;
    if (!ensure_timestamps("tasks")) return false;

    return true;
}

} // namespace taskman
