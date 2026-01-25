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
    int rc = sqlite3_open_v2(path, &db_,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                            nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "taskman: " << (db_ ? sqlite3_errmsg(db_) : "sqlite3_open") << "\n";
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        return false;
    }
    /* Attendre jusqu'à 3 s si la base est verrouillée (ex. récupération d'un -journal). */
    (void)sqlite3_exec(db_, "PRAGMA busy_timeout=3000", nullptr, nullptr, nullptr);
    return true;
}

void Database::close() {
    if (db_) {
        int rc = sqlite3_close(db_);
        if (rc != SQLITE_OK) {
            std::cerr << "taskman: sqlite3_close: " << sqlite3_errmsg(db_) << "\n";
        }
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

bool Database::run(const char* sql, const std::vector<std::optional<std::string>>& params) {
    if (!db_) {
        std::cerr << "taskman: database not open\n";
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].has_value()) {
            const std::string& s = *params[i];
            sqlite3_bind_text(stmt, static_cast<int>(i + 1), s.c_str(),
                              static_cast<int>(s.size()), SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_null(stmt, static_cast<int>(i + 1));
        }
    }
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "taskman: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }
    return true;
}

std::vector<std::map<std::string, std::optional<std::string>>> Database::query(const char* sql) {
    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (!db_) {
        std::cerr << "taskman: database not open\n";
        return rows;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db_) << "\n";
        return rows;
    }
    int ncol = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::string, std::optional<std::string>> row;
        for (int i = 0; i < ncol; ++i) {
            const char* name = sqlite3_column_name(stmt, i);
            if (!name) continue;
            if (sqlite3_column_type(stmt, i) == SQLITE_NULL) {
                row[name] = std::nullopt;
            } else {
                const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[name] = p ? std::string(p) : std::string();
            }
        }
        rows.push_back(std::move(row));
    }
    sqlite3_finalize(stmt);
    return rows;
}

std::vector<std::map<std::string, std::optional<std::string>>> Database::query(
    const char* sql, const std::vector<std::optional<std::string>>& params) {
    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (!db_) {
        std::cerr << "taskman: database not open\n";
        return rows;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db_) << "\n";
        return rows;
    }
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].has_value()) {
            const std::string& s = *params[i];
            sqlite3_bind_text(stmt, static_cast<int>(i + 1), s.c_str(),
                              static_cast<int>(s.size()), SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_null(stmt, static_cast<int>(i + 1));
        }
    }
    int ncol = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::string, std::optional<std::string>> row;
        for (int i = 0; i < ncol; ++i) {
            const char* name = sqlite3_column_name(stmt, i);
            if (!name) continue;
            if (sqlite3_column_type(stmt, i) == SQLITE_NULL) {
                row[name] = std::nullopt;
            } else {
                const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[name] = p ? std::string(p) : std::string();
            }
        }
        rows.push_back(std::move(row));
    }
    sqlite3_finalize(stmt);
    return rows;
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
