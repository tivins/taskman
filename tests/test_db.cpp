/**
 * Tests unitaires — couche DB (Database, init_schema).
 */

#include <catch2/catch_test_macros.hpp>
#include "infrastructure/db/db.hpp"

using namespace taskman;

TEST_CASE("Database::open et is_open", "[db]") {
    Database db;
    REQUIRE(!db.is_open());

    SECTION("open :memory: réussit") {
        REQUIRE(db.open(":memory:"));
        REQUIRE(db.is_open());
    }

    SECTION("open une seconde fois (déjà ouverte) retourne true") {
        REQUIRE(db.open(":memory:"));
        REQUIRE(db.open(":memory:"));
        REQUIRE(db.is_open());
    }
}

TEST_CASE("Database::close", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    db.close();
    REQUIRE(!db.is_open());
    db.close();  // no-op
    REQUIRE(!db.is_open());
}

TEST_CASE("Database::exec sans connexion", "[db]") {
    Database db;
    REQUIRE(!db.exec("CREATE TABLE t1(x INT)"));
}

TEST_CASE("Database::exec DDL", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(id INTEGER PRIMARY KEY, name TEXT)"));
    REQUIRE(db.exec("CREATE TABLE IF NOT EXISTS t1(id INTEGER)"));  // IF NOT EXISTS
}

TEST_CASE("Database::run sans connexion", "[db]") {
    Database db;
    std::vector<std::optional<std::string>> params = {"v1"};
    REQUIRE(!db.run("INSERT INTO phases (id) VALUES (?)", params));
}

TEST_CASE("Database::run avec paramètres", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(id TEXT, val TEXT)"));
    std::vector<std::optional<std::string>> params = {"id1", "valeur1"};
    REQUIRE(db.run("INSERT INTO t1(id, val) VALUES (?, ?)", params));
    params = {"id2", std::nullopt};
    REQUIRE(db.run("INSERT INTO t1(id, val) VALUES (?, ?)", params));
}

TEST_CASE("Database::run avec NULL (nullopt)", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(id TEXT, x TEXT)"));
    std::vector<std::optional<std::string>> params = {"a", std::nullopt};
    REQUIRE(db.run("INSERT INTO t1(id, x) VALUES (?, ?)", params));
    auto rows = db.query("SELECT id, x FROM t1 WHERE id = 'a'");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"].has_value());
    REQUIRE(rows[0]["id"] == "a");
    REQUIRE(!rows[0]["x"].has_value());  // NULL → nullopt
}

TEST_CASE("Database::query sans connexion", "[db]") {
    Database db;
    auto rows = db.query("SELECT 1");
    REQUIRE(rows.empty());
}

TEST_CASE("Database::query SELECT", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(id TEXT, n INT)"));
    REQUIRE(db.run("INSERT INTO t1(id, n) VALUES (?, ?)", {"a", "1"}));
    REQUIRE(db.run("INSERT INTO t1(id, n) VALUES (?, ?)", {"b", "2"}));

    auto rows = db.query("SELECT id, n FROM t1 ORDER BY id");
    REQUIRE(rows.size() == 2u);
    REQUIRE(rows[0]["id"] == "a");
    REQUIRE(rows[0]["n"] == "1");
    REQUIRE(rows[1]["id"] == "b");
    REQUIRE(rows[1]["n"] == "2");
}

TEST_CASE("Database::query noms de colonnes et NULL", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(a TEXT, b TEXT)"));
    REQUIRE(db.run("INSERT INTO t1(a, b) VALUES (?, ?)", {std::string("x"), std::nullopt}));
    auto rows = db.query("SELECT a, b FROM t1");
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0].count("a") == 1u);
    REQUIRE(rows[0].count("b") == 1u);
    REQUIRE(rows[0]["a"] == "x");
    REQUIRE(!rows[0]["b"].has_value());
}

TEST_CASE("Database::query avec paramètres", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.exec("CREATE TABLE t1(id TEXT, n INT)"));
    REQUIRE(db.run("INSERT INTO t1(id, n) VALUES (?, ?)", {"a", "1"}));
    REQUIRE(db.run("INSERT INTO t1(id, n) VALUES (?, ?)", {"b", "2"}));
    REQUIRE(db.run("INSERT INTO t1(id, n) VALUES (?, ?)", {"c", "3"}));
    auto rows = db.query("SELECT id, n FROM t1 WHERE id = ? ORDER BY id", {"b"});
    REQUIRE(rows.size() == 1u);
    REQUIRE(rows[0]["id"] == "b");
    REQUIRE(rows[0]["n"] == "2");
}

TEST_CASE("init_schema crée les 4 tables", "[db]") {
    Database db;
    REQUIRE(db.open(":memory:"));
    REQUIRE(db.init_schema());

    // Vérifier que les tables existent en interrogeant sqlite_master ou en insérant
    REQUIRE(db.exec("INSERT INTO phases (id, name) VALUES ('p1', 'Phase 1')"));
    REQUIRE(db.exec("INSERT INTO milestones (id, phase_id) VALUES ('m1', 'p1')"));
    REQUIRE(db.exec("INSERT INTO tasks (id, phase_id, title) VALUES ('t1', 'p1', 'Tâche')"));
    REQUIRE(db.exec("INSERT INTO task_deps (task_id, depends_on) VALUES ('t1', 't1')"));  // self-ref pour test
    // Nettoyage pour foreign key: en fait task_deps dépend de tasks. t1 existe.
    auto rows = db.query("SELECT name FROM sqlite_master WHERE type='table' AND name IN ('phases','milestones','tasks','task_deps')");
    REQUIRE(rows.size() == 4u);
}

TEST_CASE("init_schema échoue si DB non ouverte", "[db]") {
    Database db;
    REQUIRE(!db.init_schema());
}
