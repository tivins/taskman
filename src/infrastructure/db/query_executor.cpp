/**
 * Implémentation de QueryExecutor.
 */

#include "query_executor.hpp"
#include <sqlite3.h>
#include <iostream>

namespace taskman {

bool QueryExecutor::exec(const char* sql) {
    if (!connection_.is_open()) {
        std::cerr << "taskman: database not open\n";
        return false;
    }
    sqlite3* db = connection_.get();
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "taskman: " << (err ? err : "sqlite3_exec failed") << "\n";
        if (err) {
            sqlite3_free(err);
        }
        return false;
    }
    return true;
}

bool QueryExecutor::run(const char* sql, const std::vector<std::optional<std::string>>& params) {
    if (!connection_.is_open()) {
        std::cerr << "taskman: database not open\n";
        return false;
    }
    sqlite3* db = connection_.get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db) << "\n";
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
        std::cerr << "taskman: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

std::vector<std::map<std::string, std::optional<std::string>>> QueryExecutor::query(const char* sql) {
    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (!connection_.is_open()) {
        std::cerr << "taskman: database not open\n";
        return rows;
    }
    sqlite3* db = connection_.get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db) << "\n";
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

std::vector<std::map<std::string, std::optional<std::string>>> QueryExecutor::query(
    const char* sql, const std::vector<std::optional<std::string>>& params) {
    std::vector<std::map<std::string, std::optional<std::string>>> rows;
    if (!connection_.is_open()) {
        std::cerr << "taskman: database not open\n";
        return rows;
    }
    sqlite3* db = connection_.get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "taskman: " << sqlite3_errmsg(db) << "\n";
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

} // namespace taskman
