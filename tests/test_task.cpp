/**
 * Tests unitaires — commandes task:add, task:get, task:list, task:edit.
 */

#include <catch2/catch_test_macros.hpp>
#include "db.hpp"
#include "phase.hpp"
#include "task.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
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
    REQUIRE(init_schema(db));
    std::vector<std::string> phase_args = {"phase:add", "--id", "p1", "--name", "Conception"};
    std::vector<char*> ptrs;
    for (auto& s : phase_args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    REQUIRE(cmd_phase_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db) == 0);
}

static int run_task_add(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_task_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static std::string run_task_add_capture(Database& db, std::vector<std::string> args) {
    CoutRedirect redir;
    int r = run_task_add(db, args);
    REQUIRE(r == 0);
    return redir.str();
}

static int run_task_get(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_task_get(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static std::string run_task_get_capture(Database& db, std::vector<std::string> args) {
    CoutRedirect redir;
    int r = run_task_get(db, args);
    REQUIRE(r == 0);
    return redir.str();
}

static std::string run_task_list(Database& db, std::vector<std::string> args = {}) {
    CoutRedirect redir;
    std::vector<std::string> full = {"task:list"};
    for (auto& a : args) full.push_back(a);
    std::vector<char*> ptrs;
    for (auto& s : full) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    int r = cmd_task_list(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
    REQUIRE(r == 0);
    return redir.str();
}

static int run_task_edit(Database& db, std::vector<std::string> args) {
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_task_edit(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static int run_task_dep_add(Database& db, std::string task_id, std::string dep_id) {
    std::vector<std::string> args = {"task:dep:add", std::move(task_id), std::move(dep_id)};
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_task_dep_add(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static int run_task_dep_remove(Database& db, std::string task_id, std::string dep_id) {
    std::vector<std::string> args = {"task:dep:remove", std::move(task_id), std::move(dep_id)};
    std::vector<char*> ptrs;
    for (auto& s : args) ptrs.push_back(s.data());
    ptrs.push_back(nullptr);
    return cmd_task_dep_remove(static_cast<int>(ptrs.size() - 1), ptrs.data(), db);
}

static bool looks_like_uuid(const std::string& s) {
    // format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (36 chars, 4 hyphens)
    if (s.size() != 36) return false;
    int hyphens = 0;
    for (char c : s) {
        if (c == '-') hyphens++;
        else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {}
        else return false;
    }
    return hyphens == 4;
}

TEST_CASE("cmd_task_add — génération ID UUID v4 et sortie JSON", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "Tâche 1", "--phase", "p1"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.contains("id"));
    REQUIRE(looks_like_uuid(j["id"].get<std::string>()));
    REQUIRE(j["title"] == "Tâche 1");
    REQUIRE(j["phase_id"] == "p1");
    REQUIRE(j["status"] == "to_do");
}

TEST_CASE("cmd_task_add — avec description, role, milestone", "[task]") {
    Database db;
    setup_db(db);
    // ajouter un milestone
    REQUIRE(db.run("INSERT INTO milestones (id, phase_id, name) VALUES (?, ?, ?)",
                   {"m1", "p1", "Spec"}));
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T2", "--phase", "p1",
        "--description", "Desc", "--role", "developer", "--milestone", "m1"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j["description"] == "Desc");
    REQUIRE(j["role"] == "developer");
    REQUIRE(j["milestone_id"] == "m1");
}

TEST_CASE("cmd_task_add — --title manquant", "[task]") {
    Database db;
    setup_db(db);
    int r = run_task_add(db, {"task:add", "--phase", "p1"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_add — --phase manquant", "[task]") {
    Database db;
    setup_db(db);
    int r = run_task_add(db, {"task:add", "--title", "T"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_add — --role invalide", "[task]") {
    Database db;
    setup_db(db);
    int r = run_task_add(db, {"task:add", "--title", "T", "--phase", "p1", "--role", "invalid"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_get — trouvée, format JSON", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    auto j = nlohmann::json::parse(out);
    std::string id = j["id"].get<std::string>();
    std::string get_out = run_task_get_capture(db, {"task:get", id});
    auto j2 = nlohmann::json::parse(get_out);
    REQUIRE(j2["id"] == id);
    REQUIRE(j2["title"] == "T1");
}

TEST_CASE("cmd_task_get — non trouvée", "[task]") {
    Database db;
    setup_db(db);
    CoutRedirect cerr_redir;
    std::streambuf* cerr_prev = std::cerr.rdbuf();
    std::stringstream cerr_buf;
    std::cerr.rdbuf(cerr_buf.rdbuf());
    int r = run_task_get(db, {"task:get", "00000000-0000-0000-0000-000000000000"});
    std::cerr.rdbuf(cerr_prev);
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_list — vide", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_list(db);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.is_array());
    REQUIRE(j.empty());
}

TEST_CASE("cmd_task_list — filtre --phase", "[task]") {
    Database db;
    setup_db(db);
    REQUIRE(db.run("INSERT INTO phases (id, name) VALUES (?, ?)", {"p2", "Dev"}));
    run_task_add(db, {"task:add", "--title", "A", "--phase", "p1"});
    run_task_add(db, {"task:add", "--title", "B", "--phase", "p2"});
    run_task_add(db, {"task:add", "--title", "C", "--phase", "p1"});
    std::string out = run_task_list(db, {"--phase", "p1"});
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.size() == 2u);
    for (auto& e : j) REQUIRE(e["phase_id"] == "p1");
}

TEST_CASE("cmd_task_edit — title, status, role", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    int r = run_task_edit(db, {"task:edit", id, "--title", "T2", "--status", "in_progress", "--role", "developer"});
    REQUIRE(r == 0);
    std::string get_out = run_task_get_capture(db, {"task:get", id});
    auto j = nlohmann::json::parse(get_out);
    REQUIRE(j["title"] == "T2");
    REQUIRE(j["status"] == "in_progress");
    REQUIRE(j["role"] == "developer");
}

TEST_CASE("cmd_task_edit — id manquant", "[task]") {
    Database db;
    setup_db(db);
    int r = run_task_edit(db, {"task:edit", "--title", "X"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_edit — --status invalide", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    int r = run_task_edit(db, {"task:edit", id, "--status", "invalid"});
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_dep_add — succès", "[task]") {
    Database db;
    setup_db(db);
    std::string out1 = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    std::string out2 = run_task_add_capture(db, {"task:add", "--title", "T2", "--phase", "p1"});
    std::string id1 = nlohmann::json::parse(out1)["id"].get<std::string>();
    std::string id2 = nlohmann::json::parse(out2)["id"].get<std::string>();
    int r = run_task_dep_add(db, id1, id2);
    REQUIRE(r == 0);
    auto rows = db.query("SELECT task_id, depends_on FROM task_deps WHERE task_id = ? AND depends_on = ?", {id1, id2});
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["task_id"].value() == id1);
    REQUIRE(rows[0]["depends_on"].value() == id2);
}

TEST_CASE("cmd_task_dep_add — tâche (task-id) inexistante", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    std::stringstream sink;
    std::streambuf* prev = std::cerr.rdbuf(sink.rdbuf());
    int r = run_task_dep_add(db, "00000000-0000-0000-0000-000000000001", id);
    std::cerr.rdbuf(prev);
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_dep_add — tâche (dep-id) inexistante", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    std::stringstream sink;
    std::streambuf* prev = std::cerr.rdbuf(sink.rdbuf());
    int r = run_task_dep_add(db, id, "00000000-0000-0000-0000-000000000002");
    std::cerr.rdbuf(prev);
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_dep_add — auto-dépendance rejetée", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    std::stringstream sink;
    std::streambuf* prev = std::cerr.rdbuf(sink.rdbuf());
    int r = run_task_dep_add(db, id, id);
    std::cerr.rdbuf(prev);
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_dep_add — doublon rejeté", "[task]") {
    Database db;
    setup_db(db);
    std::string out1 = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    std::string out2 = run_task_add_capture(db, {"task:add", "--title", "T2", "--phase", "p1"});
    std::string id1 = nlohmann::json::parse(out1)["id"].get<std::string>();
    std::string id2 = nlohmann::json::parse(out2)["id"].get<std::string>();
    REQUIRE(run_task_dep_add(db, id1, id2) == 0);
    std::stringstream sink;
    std::streambuf* prev = std::cerr.rdbuf(sink.rdbuf());
    int r = run_task_dep_add(db, id1, id2);
    std::cerr.rdbuf(prev);
    REQUIRE(r == 1);
}

TEST_CASE("cmd_task_dep_remove — succès", "[task]") {
    Database db;
    setup_db(db);
    std::string out1 = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    std::string out2 = run_task_add_capture(db, {"task:add", "--title", "T2", "--phase", "p1"});
    std::string id1 = nlohmann::json::parse(out1)["id"].get<std::string>();
    std::string id2 = nlohmann::json::parse(out2)["id"].get<std::string>();
    REQUIRE(run_task_dep_add(db, id1, id2) == 0);
    int r = run_task_dep_remove(db, id1, id2);
    REQUIRE(r == 0);
    auto rows = db.query("SELECT 1 FROM task_deps WHERE task_id = ? AND depends_on = ?", {id1, id2});
    REQUIRE(rows.empty());
}

TEST_CASE("cmd_task_dep_remove — idempotent (dépendance absente)", "[task]") {
    Database db;
    setup_db(db);
    std::string out1 = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    std::string out2 = run_task_add_capture(db, {"task:add", "--title", "T2", "--phase", "p1"});
    std::string id1 = nlohmann::json::parse(out1)["id"].get<std::string>();
    std::string id2 = nlohmann::json::parse(out2)["id"].get<std::string>();
    int r = run_task_dep_remove(db, id1, id2);
    REQUIRE(r == 0);
}

TEST_CASE("cmd_task_add — --format text", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "Ma tâche", "--phase", "p1",
        "--description", "Desc", "--role", "developer", "--format", "text"});
    REQUIRE(out.find("title: Ma tâche") != std::string::npos);
    REQUIRE(out.find("description: Desc") != std::string::npos);
    REQUIRE(out.find("status: to_do") != std::string::npos);
    REQUIRE(out.find("role: developer") != std::string::npos);
    REQUIRE(out.find("id:") != std::string::npos);
}

TEST_CASE("cmd_task_get — --format text", "[task]") {
    Database db;
    setup_db(db);
    std::string out = run_task_add_capture(db, {"task:add", "--title", "T1", "--phase", "p1"});
    std::string id = nlohmann::json::parse(out)["id"].get<std::string>();
    std::string get_out = run_task_get_capture(db, {"task:get", id, "--format", "text"});
    REQUIRE(get_out.find("title: T1") != std::string::npos);
    REQUIRE(get_out.find("status: to_do") != std::string::npos);
}

TEST_CASE("cmd_task_list — --format text", "[task]") {
    Database db;
    setup_db(db);
    run_task_add(db, {"task:add", "--title", "A", "--phase", "p1"});
    run_task_add(db, {"task:add", "--title", "B", "--phase", "p1"});
    std::string out = run_task_list(db, {"--format", "text"});
    REQUIRE(out.find("title: A") != std::string::npos);
    REQUIRE(out.find("title: B") != std::string::npos);
    REQUIRE(out.find("---") != std::string::npos);
}
