/**
 * Implémentation de la couche DB.
 */

#include "db.hpp"
#include <sqlite3.h>
#include <iostream>

namespace taskman {

Database::~Database() {
    close();
}

bool Database::open(const char* path) {
    if (db_ != nullptr) {
        return true;
    }
    int rc = sqlite3_open(path, &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "taskman: " << (db_ ? sqlite3_errmsg(db_) : "sqlite3_open") << "\n";
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        return false;
    }
    return true;
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::exec(const char* sql) {
    if (!db_) {
        std::cerr << "taskman: database not open\n";
        return false;
    }
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "taskman: " << (err ? err : "sqlite3_exec failed") << "\n";
        if (err) {
            sqlite3_free(err);
        }
        return false;
    }
    return true;
}

bool init_schema(Database& db) {
    static const char* const phases_sql =
        "CREATE TABLE IF NOT EXISTS phases (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  name TEXT NOT NULL,\n"
        "  status TEXT DEFAULT 'to_do',\n"
        "  sort_order INTEGER\n"
        ");";
    if (!db.exec(phases_sql)) return false;

    static const char* const milestones_sql =
        "CREATE TABLE IF NOT EXISTS milestones (\n"
        "  id TEXT PRIMARY KEY,\n"
        "  phase_id TEXT NOT NULL,\n"
        "  name TEXT,\n"
        "  criterion TEXT,\n"
        "  reached INTEGER DEFAULT 0,\n"
        "  FOREIGN KEY (phase_id) REFERENCES phases(id)\n"
        ");";
    if (!db.exec(milestones_sql)) return false;

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
        "  FOREIGN KEY (phase_id) REFERENCES phases(id),\n"
        "  FOREIGN KEY (milestone_id) REFERENCES milestones(id)\n"
        ");";
    if (!db.exec(tasks_sql)) return false;

    static const char* const task_deps_sql =
        "CREATE TABLE IF NOT EXISTS task_deps (\n"
        "  task_id TEXT NOT NULL,\n"
        "  depends_on TEXT NOT NULL,\n"
        "  PRIMARY KEY (task_id, depends_on),\n"
        "  FOREIGN KEY (task_id) REFERENCES tasks(id),\n"
        "  FOREIGN KEY (depends_on) REFERENCES tasks(id)\n"
        ");";
    if (!db.exec(task_deps_sql)) return false;

    return true;
}

} // namespace taskman
