/**
 * Tests unitaires et d'intégration — serveur MCP (tools/list, tools/call, ping, erreurs JSON-RPC).
 * Tests d'intégration via subprocess (taskman mcp) avec pipes stdin/stdout.
 */

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
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
    const char* candidates[] = {"taskman.exe", "build/Debug/taskman.exe", "build/Release/taskman.exe", "Debug/taskman.exe", "Release/taskman.exe"};
#else
    const char* candidates[] = {"taskman", "./taskman", "build/taskman", "Release/taskman", "Debug/taskman"};
#endif
    for (const char* n : candidates) {
        if (fs::exists(n))
            return fs::absolute(fs::path(n)).string();
    }
    return "";
}

/** Exécute taskman mcp avec une requête JSON-RPC et retourne la réponse. */
static std::pair<int, std::string> run_mcp_request(const std::string& taskman_exe,
                                                    const std::string& db_path,
                                                    const std::string& request) {
#ifdef _WIN32
    _putenv_s("TASKMAN_DB_NAME", db_path.c_str());
#else
    setenv("TASKMAN_DB_NAME", db_path.c_str(), 1);
#endif

    std::string cmd = "\"" + taskman_exe + "\" mcp";
    FILE* f = popen(cmd.c_str(), "w");
    if (!f)
        return {-1, ""};

    fprintf(f, "%s\n", request.c_str());
    fflush(f);
    pclose(f);

    // Pour lire la réponse, on doit relancer avec redirection
    cmd = "echo " + request + " | \"" + taskman_exe + "\" mcp 2>&1";
#ifdef _WIN32
    // Windows: utiliser des guillemets différents pour éviter les conflits
    cmd = "echo " + request + " | \"" + taskman_exe + "\" mcp";
#endif
    f = popen(cmd.c_str(), "r");
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

/** Version améliorée qui utilise un pipe bidirectionnel. */
static std::string run_mcp_request_bidirectional(const std::string& taskman_exe,
                                                const std::string& db_path,
                                                const std::string& request) {
#ifdef _WIN32
    _putenv_s("TASKMAN_DB_NAME", db_path.c_str());
#else
    setenv("TASKMAN_DB_NAME", db_path.c_str(), 1);
#endif

    // Utiliser un fichier temporaire pour la requête
    std::string tmp_req = (fs::temp_directory_path() / "mcp_req.json").string();
    std::ofstream req_file(tmp_req);
    req_file << request << "\n";
    req_file.close();

    std::string cmd;
#ifdef _WIN32
    cmd = "type \"" + tmp_req + "\" | \"" + taskman_exe + "\" mcp";
#else
    cmd = "cat \"" + tmp_req + "\" | \"" + taskman_exe + "\" mcp";
#endif

    FILE* f = popen(cmd.c_str(), "r");
    std::string out;
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f))
            out += buf;
        pclose(f);
    }

    fs::remove(tmp_req);
    return out;
}

TEST_CASE("MCP — initialize", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found (build taskman first, or set TASKMAN_EXE)");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_1.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 1;
    req["method"] = "initialize";
    req["params"]["protocolVersion"] = "2025-11-25";
    req["params"]["capabilities"] = nlohmann::json::object();
    req["params"]["clientInfo"]["name"] = "test";
    req["params"]["clientInfo"]["version"] = "1.0";

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 1);
    REQUIRE(resp.contains("result"));
    REQUIRE(resp["result"]["protocolVersion"] == "2025-11-25");
    REQUIRE(resp["result"]["serverInfo"]["name"] == "taskman");
    REQUIRE(resp["result"]["serverInfo"].contains("version"));
}

TEST_CASE("MCP — initialize avec version non supportée", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_2.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 2;
    req["method"] = "initialize";
    req["params"]["protocolVersion"] = "2024-11-05";  // Version non supportée
    req["params"]["capabilities"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 2);
    REQUIRE(resp.contains("error"));
    REQUIRE(resp["error"]["code"] == -32602);
    REQUIRE(resp["error"]["message"] == "Unsupported protocol version");
}

TEST_CASE("MCP — tools/list", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_3.db").string();
    fs::remove(db);

    // D'abord initialize
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 1;
        req["method"] = "initialize";
        req["params"]["protocolVersion"] = "2025-11-25";
        req["params"]["capabilities"] = nlohmann::json::object();
        run_mcp_request_bidirectional(exe, db, req.dump());
    }

    // Puis notifications/initialized
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["method"] = "notifications/initialized";
        run_mcp_request_bidirectional(exe, db, req.dump());
    }

    // Ensuite tools/list
    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 3;
    req["method"] = "tools/list";
    req["params"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 3);
    REQUIRE(resp.contains("result"));
    REQUIRE(resp["result"].contains("tools"));
    REQUIRE(resp["result"]["tools"].is_array());
    REQUIRE(resp["result"]["tools"].size() == 20u);

    // Vérifier quelques outils
    bool found_init = false, found_phase_add = false, found_task_list = false, found_demo_generate = false;
    for (const auto& tool : resp["result"]["tools"]) {
        std::string name = tool["name"];
        if (name == "taskman_init") found_init = true;
        if (name == "taskman_phase_add") found_phase_add = true;
        if (name == "taskman_task_list") found_task_list = true;
        if (name == "taskman_demo_generate") found_demo_generate = true;
        REQUIRE(tool.contains("description"));
        REQUIRE(tool.contains("inputSchema"));
    }
    REQUIRE(found_init);
    REQUIRE(found_phase_add);
    REQUIRE(found_task_list);
    REQUIRE(found_demo_generate);
}

TEST_CASE("MCP — ping", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_4.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 4;
    req["method"] = "ping";
    req["params"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 4);
    REQUIRE(resp.contains("result"));
    REQUIRE(resp["result"].is_object());
}

TEST_CASE("MCP — tools/call taskman_init", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_5.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 5;
    req["method"] = "tools/call";
    req["params"]["name"] = "taskman_init";
    req["params"]["arguments"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 5);
    REQUIRE(resp.contains("result"));
    REQUIRE(resp["result"].contains("content"));
    REQUIRE(resp["result"]["content"].is_array());
    REQUIRE(resp["result"]["content"].size() == 1u);
    REQUIRE(resp["result"]["content"][0]["type"] == "text");
    REQUIRE(resp["result"]["isError"] == false);
}

TEST_CASE("MCP — tools/call taskman_phase_list (vide)", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_6.db").string();
    fs::remove(db);

    // Init d'abord
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 1;
        req["method"] = "tools/call";
        req["params"]["name"] = "taskman_init";
        req["params"]["arguments"] = nlohmann::json::object();
        run_mcp_request_bidirectional(exe, db, req.dump());
    }

    // Puis phase:list
    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 6;
    req["method"] = "tools/call";
    req["params"]["name"] = "taskman_phase_list";
    req["params"]["arguments"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 6);
    REQUIRE(resp.contains("result"));
    REQUIRE(resp["result"]["content"][0]["type"] == "text");
    std::string text = resp["result"]["content"][0]["text"];
    auto phases = nlohmann::json::parse(text);
    REQUIRE(phases.is_array());
    REQUIRE(phases.empty());
}

TEST_CASE("MCP — tools/call taskman_phase_add puis list", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_7.db").string();
    fs::remove(db);

    // Init
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 1;
        req["method"] = "tools/call";
        req["params"]["name"] = "taskman_init";
        req["params"]["arguments"] = nlohmann::json::object();
        run_mcp_request_bidirectional(exe, db, req.dump());
    }

    // phase:add
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 7;
        req["method"] = "tools/call";
        req["params"]["name"] = "taskman_phase_add";
        req["params"]["arguments"]["id"] = "P1";
        req["params"]["arguments"]["name"] = "Phase 1";
        req["params"]["arguments"]["status"] = "to_do";

        std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
        auto resp = nlohmann::json::parse(response);
        REQUIRE(resp["result"]["isError"] == false);
    }

    // phase:list
    {
        nlohmann::json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 8;
        req["method"] = "tools/call";
        req["params"]["name"] = "taskman_phase_list";
        req["params"]["arguments"] = nlohmann::json::object();

        std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
        auto resp = nlohmann::json::parse(response);
        REQUIRE(resp["result"]["isError"] == false);
        std::string text = resp["result"]["content"][0]["text"];
        auto phases = nlohmann::json::parse(text);
        REQUIRE(phases.size() == 1u);
        REQUIRE(phases[0]["id"] == "P1");
        REQUIRE(phases[0]["name"] == "Phase 1");
    }
}

TEST_CASE("MCP — tools/call avec outil inconnu", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_8.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 9;
    req["method"] = "tools/call";
    req["params"]["name"] = "unknown_tool";
    req["params"]["arguments"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 9);
    REQUIRE(resp.contains("error"));
    REQUIRE(resp["error"]["code"] == -32602);
    REQUIRE(resp["error"]["message"].get<std::string>().find("Unknown tool") != std::string::npos);
}

TEST_CASE("MCP — tools/call avec arguments invalides", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_9.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 10;
    req["method"] = "tools/call";
    req["params"]["name"] = "taskman_phase_add";
    req["params"]["arguments"] = "invalid";  // Pas un objet

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp.contains("error"));
    REQUIRE(resp["error"]["code"] == -32602);
}

TEST_CASE("MCP — méthode inconnue", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_10.db").string();
    fs::remove(db);

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["id"] = 11;
    req["method"] = "unknown/method";
    req["params"] = nlohmann::json::object();

    std::string response = run_mcp_request_bidirectional(exe, db, req.dump());
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp["jsonrpc"] == "2.0");
    REQUIRE(resp["id"] == 11);
    REQUIRE(resp.contains("error"));
    REQUIRE(resp["error"]["code"] == -32601);
    REQUIRE(resp["error"]["message"] == "Method not found");
}

TEST_CASE("MCP — parse error (JSON invalide)", "[mcp][integration]") {
    std::string exe = get_taskman_path();
    if (exe.empty())
        SKIP("taskman executable not found");

    std::string db = (fs::temp_directory_path() / "taskman_mcp_11.db").string();
    fs::remove(db);

    std::string invalid_json = "{ invalid json }";
    std::string response = run_mcp_request_bidirectional(exe, db, invalid_json);
    REQUIRE(!response.empty());

    auto resp = nlohmann::json::parse(response);
    REQUIRE(resp.contains("error"));
    REQUIRE(resp["error"]["code"] == -32700);
    REQUIRE(resp["error"]["message"] == "Parse error");
}
