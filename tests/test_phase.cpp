/**
 * Tests unitaires — commandes phase:add, phase:edit, phase:list.
 */

#include <catch2/catch_test_macros.hpp>
#include "db.hpp"
#include "phase.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>

using namespace taskman;

// Redirection de std::cout pour capturer la sortie de phase:list.
struct CoutRedirect {
    std::streambuf* prev;
    std::stringstream buf;
    CoutRedirect() : prev(std::cout.rdbuf()) { std::cout.rdbuf(buf.rdbuf()); }
    ~CoutRedirect() { std::cout.rdbuf(prev); }
    std::string str() const { return buf.str(); }
};

static void setup_db(taskman::Database& db) {
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.init_schema());
}

// Construire argv pour éviter les avertissements literal -> char*.
static int run_phase_add(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return taskman::cmd_phase_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static int run_phase_edit(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return taskman::cmd_phase_edit(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static std::string run_phase_list(Database& db) {
    CoutRedirect redir;
    char* argv[] = {const_cast<char*>("phase:list"), nullptr};
    int r = taskman::cmd_phase_list(1, argv, db);
    REQUIRE(r == 0);
    return redir.str();
}

TEST_CASE("phase_add — succès (utilitaire)", "[phase]") {
    Database db;
    setup_db(db);
    REQUIRE(phase_add(db, "p1", "Conception", "to_do", std::nullopt));
    auto rows = db.query("SELECT id, name, status, sort_order FROM phases WHERE id = 'p1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "p1");
    REQUIRE(rows[0]["name"] == "Conception");
    REQUIRE(rows[0]["status"] == "to_do");
    REQUIRE(!rows[0]["sort_order"].has_value());
}

TEST_CASE("phase_add — avec sort_order (utilitaire)", "[phase]") {
    Database db;
    setup_db(db);
    REQUIRE(phase_add(db, "p2", "Dev", "in_progress", 42));
    auto rows = db.query("SELECT id, name, status, sort_order FROM phases WHERE id = 'p2'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["status"] == "in_progress");
    REQUIRE(rows[0]["sort_order"] == "42");
}

TEST_CASE("phase_add — status invalide (utilitaire)", "[phase]") {
    Database db;
    setup_db(db);
    REQUIRE(!phase_add(db, "p1", "X", "invalid", std::nullopt));
    auto rows = db.query("SELECT 1 FROM phases WHERE id = 'p1'");
    REQUIRE(rows.empty());
}

TEST_CASE("cmd_phase_add — succès", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--id", "p1", "--name", "Conception"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, name, status, sort_order FROM phases WHERE id = 'p1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "p1");
    REQUIRE(rows[0]["name"] == "Conception");
    REQUIRE(rows[0]["status"] == "to_do");  // défaut
    REQUIRE(!rows[0]["sort_order"].has_value());  // NULL
}

TEST_CASE("cmd_phase_add — avec status et sort-order", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--id", "p2", "--name", "Dev", "--status", "in_progress", "--sort-order", "42"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, name, status, sort_order FROM phases WHERE id = 'p2'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["status"] == "in_progress");
    REQUIRE(rows[0]["sort_order"] == "42");
}

TEST_CASE("cmd_phase_add — --id manquant", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--name", "X"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_phase_add — --name manquant", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--id", "p1"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_phase_add — --status invalide", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--id", "p1", "--name", "X", "--status", "invalid"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_phase_add — --sort-order non entier", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_add(db, {"phase:add", "--id", "p1", "--name", "X", "--sort-order", "abc"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_phase_edit — id manquant", "[phase]") {
    Database db;
    setup_db(db);
    int r = run_phase_edit(db, {"phase:edit", "--name", "Y"});  // pas de positionnel id
    REQUIRE(r == 1);
}

TEST_CASE("cmd_phase_edit — rien à mettre à jour", "[phase]") {
    Database db;
    setup_db(db);
    run_phase_add(db, {"phase:add", "--id", "p1", "--name", "A"});
    int r = run_phase_edit(db, {"phase:edit", "p1"});  // aucun --name, --status, --sort-order
    REQUIRE(r == 0);
}

TEST_CASE("cmd_phase_edit — mise à jour name, status, sort_order", "[phase]") {
    Database db;
    setup_db(db);
    run_phase_add(db, {"phase:add", "--id", "p1", "--name", "A", "--sort-order", "1"});
    int r = run_phase_edit(db, {"phase:edit", "p1", "--name", "A2", "--status", "done", "--sort-order", "10"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, name, status, sort_order FROM phases WHERE id = 'p1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["name"] == "A2");
    REQUIRE(rows[0]["status"] == "done");
    REQUIRE(rows[0]["sort_order"] == "10");
}

TEST_CASE("cmd_phase_list — vide", "[phase]") {
    Database db;
    setup_db(db);
    std::string out = run_phase_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.is_array());
    REQUIRE(j.empty());
}

TEST_CASE("cmd_phase_list — une phase", "[phase]") {
    Database db;
    setup_db(db);
    run_phase_add(db, {"phase:add", "--id", "p1", "--name", "Phase 1", "--status", "done"});
    std::string out = run_phase_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 1u);
    REQUIRE(j[0]["id"] == "p1");
    REQUIRE(j[0]["name"] == "Phase 1");
    REQUIRE(j[0]["status"] == "done");
    REQUIRE(j[0]["sort_order"].is_null());  // NULL → null en JSON
}

TEST_CASE("cmd_phase_list — sort_order formaté en nombre", "[phase]") {
    Database db;
    setup_db(db);
    run_phase_add(db, {"phase:add", "--id", "p1", "--name", "N", "--sort-order", "7"});
    std::string out = run_phase_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 1u);
    REQUIRE(j[0]["sort_order"].is_number_integer());
    REQUIRE(j[0]["sort_order"] == 7);
}

TEST_CASE("cmd_phase_list — ordre par sort_order", "[phase]") {
    Database db;
    setup_db(db);
    run_phase_add(db, {"phase:add", "--id", "p2", "--name", "Deux", "--sort-order", "2"});
    run_phase_add(db, {"phase:add", "--id", "p1", "--name", "Un", "--sort-order", "1"});
    run_phase_add(db, {"phase:add", "--id", "p3", "--name", "Trois", "--sort-order", "3"});
    std::string out = run_phase_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 3u);
    REQUIRE(j[0]["id"] == "p1");
    REQUIRE(j[1]["id"] == "p2");
    REQUIRE(j[2]["id"] == "p3");
}
