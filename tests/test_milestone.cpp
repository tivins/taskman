/**
 * Tests unitaires — commandes milestone:add, milestone:edit, milestone:list.
 */

#include <catch2/catch_test_macros.hpp>
#include "infrastructure/db/db.hpp"
#include "core/milestone/milestone.hpp"
#include "core/phase/phase.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>

using namespace taskman;

struct CoutRedirect {
    std::streambuf* prev;
    std::stringstream buf;
    CoutRedirect() : prev(std::cout.rdbuf()) { std::cout.rdbuf(buf.rdbuf()); }
    ~CoutRedirect() { std::cout.rdbuf(prev); }
    std::string str() const { return buf.str(); }
};

static void setup_db(Database& db) {
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.init_schema());
    // Ajouter une phase (FK milestone.phase_id)
    std::vector<char*> add_phase_argv;
    std::vector<std::string> phase_args = {"phase:add", "--id", "p1", "--name", "Conception"};
    for (auto& s : phase_args) add_phase_argv.push_back(s.data());
    add_phase_argv.push_back(nullptr);
    REQUIRE(cmd_phase_add(static_cast<int>(add_phase_argv.size() - 1), add_phase_argv.data(), db) == 0);
}

static int run_milestone_add(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_milestone_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static int run_phase_add(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_phase_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static int run_milestone_edit(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_milestone_edit(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static std::string run_milestone_list(Database& db, std::vector<std::string> args = {}) {
    CoutRedirect redir;
    std::vector<std::string> full = {"milestone:list"};
    for (auto& a : args) full.push_back(a);
    std::vector<char*> ptrs;
    for (auto& s : full) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    int r = cmd_milestone_list(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
    REQUIRE(r == 0);
    return redir.str();
}

TEST_CASE("milestone_add — succès (utilitaire)", "[milestone]") {
    Database db;
    setup_db(db);
    REQUIRE(milestone_add(db, "m1", "p1", std::nullopt, std::nullopt, false));
    auto rows = db.query("SELECT id, phase_id, name, criterion, reached FROM milestones WHERE id = 'm1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "m1");
    REQUIRE(rows[0]["phase_id"] == "p1");
    REQUIRE(!rows[0]["name"].has_value());
    REQUIRE(!rows[0]["criterion"].has_value());
    REQUIRE(rows[0]["reached"] == "0");
}

TEST_CASE("milestone_add — avec name, criterion, reached (utilitaire)", "[milestone]") {
    Database db;
    setup_db(db);
    REQUIRE(milestone_add(db, "m2", "p1", "Spec validée", "Doc approuvé", true));
    auto rows = db.query("SELECT id, phase_id, name, criterion, reached FROM milestones WHERE id = 'm2'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["name"] == "Spec validée");
    REQUIRE(rows[0]["criterion"] == "Doc approuvé");
    REQUIRE(rows[0]["reached"] == "1");
}

TEST_CASE("cmd_milestone_add — succès (id, phase)", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, phase_id, name, criterion, reached FROM milestones WHERE id = 'm1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "m1");
    REQUIRE(rows[0]["phase_id"] == "p1");
    REQUIRE(!rows[0]["name"].has_value());
    REQUIRE(!rows[0]["criterion"].has_value());
    REQUIRE(rows[0]["reached"] == "0");
}

TEST_CASE("cmd_milestone_add — avec name, criterion, reached", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_add(db, {"milestone:add", "--id", "m2", "--phase", "p1",
        "--name", "Spec validée", "--criterion", "Doc approuvé", "--reached", "1"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, phase_id, name, criterion, reached FROM milestones WHERE id = 'm2'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["name"] == "Spec validée");
    REQUIRE(rows[0]["criterion"] == "Doc approuvé");
    REQUIRE(rows[0]["reached"] == "1");
}

TEST_CASE("cmd_milestone_add — --id manquant", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_add(db, {"milestone:add", "--phase", "p1"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_milestone_add — --phase manquant", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_add(db, {"milestone:add", "--id", "m1"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_milestone_add — --reached invalide", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1", "--reached", "2"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_milestone_edit — id manquant", "[milestone]") {
    Database db;
    setup_db(db);
    int r = run_milestone_edit(db, {"milestone:edit", "--name", "Y"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_milestone_edit — rien à mettre à jour", "[milestone]") {
    Database db;
    setup_db(db);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    int r = run_milestone_edit(db, {"milestone:edit", "m1"});
    REQUIRE(r == 0);
}

TEST_CASE("cmd_milestone_edit — name, criterion, reached", "[milestone]") {
    Database db;
    setup_db(db);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1", "--name", "A"});
    int r = run_milestone_edit(db, {"milestone:edit", "m1", "--name", "A2", "--criterion", "Crit", "--reached", "1"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT id, name, criterion, reached FROM milestones WHERE id = 'm1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["name"] == "A2");
    REQUIRE(rows[0]["criterion"] == "Crit");
    REQUIRE(rows[0]["reached"] == "1");
}

TEST_CASE("cmd_milestone_edit — --phase (changement de phase)", "[milestone]") {
    Database db;
    setup_db(db);
    REQUIRE(run_phase_add(db, {"phase:add", "--id", "p2", "--name", "Dev"}) == 0);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    int r = run_milestone_edit(db, {"milestone:edit", "m1", "--phase", "p2"});
    REQUIRE(r == 0);
    auto rows = db.query("SELECT phase_id FROM milestones WHERE id = 'm1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["phase_id"] == "p2");
}

TEST_CASE("cmd_milestone_edit — --reached invalide", "[milestone]") {
    Database db;
    setup_db(db);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    int r = run_milestone_edit(db, {"milestone:edit", "m1", "--reached", "x"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_milestone_list — vide", "[milestone]") {
    Database db;
    setup_db(db);
    std::string out = run_milestone_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.is_array());
    REQUIRE(j.empty());
}

TEST_CASE("cmd_milestone_list — un milestone", "[milestone]") {
    Database db;
    setup_db(db);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1", "--name", "M1", "--reached", "1"});
    std::string out = run_milestone_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 1u);
    REQUIRE(j[0]["id"] == "m1");
    REQUIRE(j[0]["phase_id"] == "p1");
    REQUIRE(j[0]["name"] == "M1");
    REQUIRE(j[0]["reached"].is_number_integer());
    REQUIRE(j[0]["reached"] == 1);
}

TEST_CASE("cmd_milestone_list — reached et champs null en JSON", "[milestone]") {
    Database db;
    setup_db(db);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});  // name, criterion NULL
    std::string out = run_milestone_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 1u);
    REQUIRE(j[0]["reached"].is_number_integer());
    REQUIRE(j[0]["reached"] == 0);
    REQUIRE(j[0]["name"].is_null());
    REQUIRE(j[0]["criterion"].is_null());
}

TEST_CASE("cmd_milestone_list — filtre --phase", "[milestone]") {
    Database db;
    setup_db(db);
    REQUIRE(run_phase_add(db, {"phase:add", "--id", "p2", "--name", "Dev"}) == 0);
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    run_milestone_add(db, {"milestone:add", "--id", "m2", "--phase", "p2"});
    run_milestone_add(db, {"milestone:add", "--id", "m3", "--phase", "p1"});
    std::string out = run_milestone_list(db, {"--phase", "p1"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 2u);
    REQUIRE((j[0]["id"] == "m1" || j[0]["id"] == "m3"));
    REQUIRE((j[1]["id"] == "m1" || j[1]["id"] == "m3"));
    REQUIRE(j[0]["phase_id"] == "p1");
    REQUIRE(j[1]["phase_id"] == "p1");
}

TEST_CASE("cmd_milestone_list — ordre phase_id, id", "[milestone]") {
    Database db;
    setup_db(db);
    REQUIRE(run_phase_add(db, {"phase:add", "--id", "p2", "--name", "Dev"}) == 0);
    run_milestone_add(db, {"milestone:add", "--id", "m2", "--phase", "p2"});
    run_milestone_add(db, {"milestone:add", "--id", "m1", "--phase", "p1"});
    std::string out = run_milestone_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 2u);
    REQUIRE(j[0]["id"] == "m1");  // p1 avant p2
    REQUIRE(j[1]["id"] == "m2");
}
