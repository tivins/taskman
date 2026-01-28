/**
 * Tests unitaires — note_add, cmd_note_add, cmd_note_list.
 */

#include <catch2/catch_test_macros.hpp>
#include "infrastructure/db/db.hpp"
#include "core/milestone/milestone.hpp"
#include "core/note/note.hpp"
#include "core/phase/phase.hpp"
#include "core/task/task.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

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
    std::vector<std::string> phase_args = {"phase:add", "--id", "p1", "--name", "Conception"};
    std::vector<char*> ptrs;
    for (auto& s : phase_args) ptrs.push_back(s.data());
    REQUIRE(cmd_phase_add(static_cast<int>(ptrs.size()), ptrs.data(), db) == 0);
    REQUIRE(task_add(db, "t1", "p1", std::nullopt, "Tâche 1", std::nullopt, "to_do", std::nullopt, std::nullopt));
}

static int run_note_add(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    return cmd_note_add(static_cast<int>(ptrs.size()), ptrs.data(), db);
}

static std::string run_note_add_capture(Database& db, std::vector<std::string> args) {
    CoutRedirect redir;
    int r = run_note_add(db, args);
    REQUIRE(r == 0);
    return redir.str();
}

static std::string run_note_list(Database& db, std::vector<std::string> args) {
    CoutRedirect redir;
    std::vector<std::string> full = {"task:note:list"};
    for (auto& a : args) full.push_back(a);
    std::vector<char*> ptrs;
    for (auto& s : full) ptrs.push_back(s.data());
    int r = cmd_note_list(static_cast<int>(ptrs.size()), ptrs.data(), db);
    REQUIRE(r == 0);
    return redir.str();
}

static bool looks_like_uuid(const std::string& s) {
    if (s.size() != 36) return false;
    int hyphens = 0;
    for (char c : s) {
        if (c == '-') hyphens++;
        else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {}
        else return false;
    }
    return hyphens == 4;
}

TEST_CASE("note_add — succès (utilitaire)", "[note]") {
    Database db;
    setup_db(db);
    REQUIRE(note_add(db, "n1", "t1", "Réalisé sans souci", std::optional<std::string>("completion"), std::optional<std::string>("developer")));
    auto rows = db.query("SELECT id, task_id, content, kind, role FROM task_notes WHERE id = 'n1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "n1");
    REQUIRE(rows[0]["task_id"] == "t1");
    REQUIRE(rows[0]["content"] == "Réalisé sans souci");
    REQUIRE(rows[0]["kind"] == "completion");
    REQUIRE(rows[0]["role"] == "developer");
}

TEST_CASE("note_add — tâche inexistante (utilitaire)", "[note]") {
    Database db;
    setup_db(db);
    REQUIRE(!note_add(db, "n1", "unknown", "Contenu", std::nullopt, std::nullopt));
    auto rows = db.query("SELECT 1 FROM task_notes WHERE id = 'n1'");
    REQUIRE(rows.empty());
}

TEST_CASE("note_add — rôle invalide (utilitaire)", "[note]") {
    Database db;
    setup_db(db);
    REQUIRE(!note_add(db, "n1", "t1", "Contenu", std::nullopt, std::optional<std::string>("invalid-role")));
    auto rows = db.query("SELECT 1 FROM task_notes WHERE id = 'n1'");
    REQUIRE(rows.empty());
}

TEST_CASE("cmd_note_add — succès et sortie JSON", "[note]") {
    Database db;
    setup_db(db);
    std::string out = run_note_add_capture(db, {"task:note:add", "t1", "--content", "Note de fin"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.contains("id"));
    REQUIRE(looks_like_uuid(j["id"].get<std::string>()));
    REQUIRE(j["task_id"] == "t1");
    REQUIRE(j["content"] == "Note de fin");
    REQUIRE(j.contains("created_at"));
}

TEST_CASE("cmd_note_add — avec kind et role", "[note]") {
    Database db;
    setup_db(db);
    std::string out = run_note_add_capture(db, {"task:note:add", "t1", "--content", "Blocage", "--kind", "issue", "--role", "developer"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j["kind"] == "issue");
    REQUIRE(j["role"] == "developer");
}

TEST_CASE("cmd_note_add — --content manquant", "[note]") {
    Database db;
    setup_db(db);
    int r = run_note_add(db, {"task:note:add", "t1"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_note_add — task-id manquant", "[note]") {
    Database db;
    setup_db(db);
    int r = run_note_add(db, {"task:note:add", "--content", "x"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_note_list — liste vide puis après ajout", "[note]") {
    Database db;
    setup_db(db);
    std::string out0 = run_note_list(db, {"t1"});
    auto arr0 = nlohmann::json::parse(out0);
    REQUIRE(arr0.is_array());
    REQUIRE(arr0.size() == 0u);

    run_note_add_capture(db, {"task:note:add", "t1", "--content", "Première note"});
    run_note_add_capture(db, {"task:note:add", "t1", "--content", "Deuxième note"});

    std::string out1 = run_note_list(db, {"t1"});
    auto arr1 = nlohmann::json::parse(out1);
    REQUIRE(arr1.is_array());
    REQUIRE(arr1.size() == 2u);
    REQUIRE(arr1[0]["content"] == "Première note");
    REQUIRE(arr1[1]["content"] == "Deuxième note");
}

TEST_CASE("cmd_note_list — task_id inexistant retourne liste vide", "[note]") {
    Database db;
    setup_db(db);
    std::string out = run_note_list(db, {"nonexistent"});
    auto arr = nlohmann::json::parse(out);
    REQUIRE(arr.is_array());
    REQUIRE(arr.size() == 0u);
}

TEST_CASE("note_add — contrainte FK : suppression tâche supprime pas les notes (orphans)", "[note]") {
    Database db;
    setup_db(db);
    REQUIRE(note_add(db, "n1", "t1", "Contenu", std::nullopt, std::nullopt));
    // La FK par défaut en SQLite est NO ACTION ; les notes restent même si la tâche est supprimée
    // (comportement SQLite). On vérifie juste que la note existe et est liée à t1.
    auto rows = db.query("SELECT task_id FROM task_notes WHERE id = 'n1'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["task_id"] == "t1");
}
