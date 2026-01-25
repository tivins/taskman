/**
 * Tests d'intégration — appels `taskman` en subprocess, fixtures DB.
 * Nécessite que l'exécutable taskman soit buildé (même répertoire, Release/, ou TASKMAN_EXE).
 */

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#else
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

static std::string get_taskman_path() {
    const char* env = std::getenv("TASKMAN_EXE");
    if (env && env[0] != '\0') {
        fs::path p(env);
        if (p.is_relative())
            p = fs::current_path() / p;
        if (fs::exists(p))
            return fs::absolute(p).string();
    }
#ifdef _WIN32
    const char* candidates[] = {"taskman.exe", "Release/taskman.exe", "Debug/taskman.exe"};
#else
    const char* candidates[] = {"taskman", "./taskman", "Release/taskman", "Debug/taskman"};
#endif
    for (const char* n : candidates) {
        if (fs::exists(n))
            return fs::absolute(fs::path(n)).string();
    }
    return "";
}

static std::pair<int, std::string> run_taskman(const std::string& taskman_exe,
                                               const std::string& db_path,
                                               const std::vector<std::string>& args) {
#ifdef _WIN32
    _putenv_s("TASKMAN_DB_NAME", db_path.c_str());
#else
    setenv("TASKMAN_DB_NAME", db_path.c_str(), 1);
#endif
    std::string cmd;
    auto quote = [](const std::string& s) {
        if (s.find(' ') != std::string::npos || s.find('"') != std::string::npos)
            return "\"" + s + "\"";
        return s;
    };
    // Exe: toujours quoté (chemins absolus avec \ sous Windows)
    cmd = "\"" + taskman_exe + "\"";
    for (const auto& a : args)
        cmd += " " + quote(a);
    cmd += " 2>&1";

    FILE* f = popen(cmd.c_str(), "r");
    std::string out;
    int status = -1;
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f))
            out += buf;
#ifdef _WIN32
        status = static_cast<int>(pclose(f));
#else
        int w = pclose(f);
        status = WIFEXITED(w) ? WEXITSTATUS(w) : -1;
#endif
    }
    return {status, out};
}

TEST_CASE("integration — init puis phase:list (vide)", "[integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found (build taskman first, or set TASKMAN_EXE)");

    std::string db = (fs::temp_directory_path() / "taskman_int_1.db").string();
    fs::remove(db);

    int code;
    std::string out;
    std::tie(code, out) = run_taskman(exe, db, {"init"});
    REQUIRE(code == 0);

    std::tie(code, out) = run_taskman(exe, db, {"phase:list"});
    REQUIRE(code == 0);
    auto j = nlohmann::json::parse(out);
    REQUIRE(j.is_array());
    REQUIRE(j.empty());
}

TEST_CASE("integration — init, phase:add, phase:list", "[integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found (build taskman first, or set TASKMAN_EXE)");

    std::string db = (fs::temp_directory_path() / "taskman_int_2.db").string();
    fs::remove(db);

    int code;
    std::string out;
    std::tie(code, out) = run_taskman(exe, db, {"init"});
    REQUIRE(code == 0);

    std::tie(code, out) = run_taskman(exe, db, {"phase:add", "--id", "p1", "--name", "Phase1"});
    REQUIRE(code == 0);

    std::tie(code, out) = run_taskman(exe, db, {"phase:list"});
    REQUIRE(code == 0);
    REQUIRE(out.find("Phase1") != std::string::npos);
    REQUIRE(out.find("p1") != std::string::npos);
}

TEST_CASE("integration — init, phase:add, task:add, task:get", "[integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found (build taskman first, or set TASKMAN_EXE)");

    std::string db = (fs::temp_directory_path() / "taskman_int_3.db").string();
    fs::remove(db);

    int code;
    std::string out;
    std::tie(code, out) = run_taskman(exe, db, {"init"});
    REQUIRE(code == 0);

    std::tie(code, out) = run_taskman(exe, db, {"phase:add", "--id", "p1", "--name", "P1"});
    REQUIRE(code == 0);

    std::tie(code, out) = run_taskman(exe, db, {"task:add", "--title", "T1", "--phase", "p1"});
    REQUIRE(code == 0);
    auto j = nlohmann::json::parse(out);
    std::string id = j["id"].get<std::string>();

    std::tie(code, out) = run_taskman(exe, db, {"task:get", id});
    REQUIRE(code == 0);
    auto j2 = nlohmann::json::parse(out);
    REQUIRE(j2["title"] == "T1");
    REQUIRE(j2["phase_id"] == "p1");
}

TEST_CASE("integration — commande inconnue → exit 1", "[integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found (build taskman first, or set TASKMAN_EXE)");

    std::string db = (fs::temp_directory_path() / "taskman_int_4.db").string();
    fs::remove(db);
    std::tie(std::ignore, std::ignore) = run_taskman(exe, db, {"init"});

    int code;
    std::string out;
    std::tie(code, out) = run_taskman(exe, db, {"unknown:command"});
    REQUIRE(code == 1);
    REQUIRE(out.find("Unknown command") != std::string::npos);
}
