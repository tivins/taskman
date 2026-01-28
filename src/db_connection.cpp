/**
 * Implémentation de DatabaseConnection.
 */

#include "db_connection.hpp"
#include <sqlite3.h>
#include <cstdlib>
#include <iostream>

namespace taskman {

namespace {

/** True if TASKMAN_JOURNAL_MEMORY is "1", or if CURSOR_AGENT is set (any value).
 * Avoids creating -journal on disk; helps when the sandbox (e.g. Cursor agent) blocks it. */
bool use_memory_journal() {
    const char* j = std::getenv("TASKMAN_JOURNAL_MEMORY");
    if (j && j[0] == '1' && j[1] == '\0') return true;
    if (std::getenv("CURSOR_AGENT") != nullptr) return true;
    return false;
}

} // namespace

DatabaseConnection::~DatabaseConnection() {
    close();
}

bool DatabaseConnection::open(const char* path) {
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
    if (use_memory_journal()) {
        (void)sqlite3_exec(db_, "PRAGMA journal_mode=MEMORY", nullptr, nullptr, nullptr);
    }
    return true;
}

void DatabaseConnection::close() {
    if (db_) {
        int rc = sqlite3_close(db_);
        if (rc != SQLITE_OK) {
            std::cerr << "taskman: sqlite3_close: " << sqlite3_errmsg(db_) << "\n";
        }
        db_ = nullptr;
    }
}

} // namespace taskman
