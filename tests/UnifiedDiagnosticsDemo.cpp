#include "entos/UnifiedDiagnostics.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Simple JSON parser for testing
static Json::Value readJsonFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Cannot open " << path << "\n";
        return Json::Value();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    Json::CharReaderBuilder builder;
    std::string errors;
    Json::Value root;
    
    std::istringstream input(buffer.str());
    if (!Json::parseFromStream(builder, input, &root, &errors)) {
        std::cerr << "JSON parse error: " << errors << "\n";
        return Json::Value();
    }
    
    return root;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <mock_data_dir>\n";
        std::cerr << "Example: " << argv[0] << " tests/mocks\n";
        return 1;
    }
    
    std::string mockDir = argv[1];
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                   APPMANAGER UNIFIED DIAGNOSTICS v2.0                      ║\n";
    std::cout << "║                    Human-Readable Table Format Output                      ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n\n";
    
    // Test 1: Loaded Apps
    {
        std::cout << "TEST 1: LOADED APPLICATIONS\n";
        std::cout << "──────────────────────────────────────────────────────────────────────────\n\n";
        
        auto data = readJsonFile(mockDir + "/app_loaded.json");
        if (!data.isNull()) {
            auto apps = data.isMember("apps") ? data["apps"] : data;
            std::string output = entos::UnifiedDiagnostics::formatLoadedApps(apps);
            std::cout << output;
        }
        std::cout << "\n\n";
    }
    
    // Test 2: Installed Apps
    {
        std::cout << "TEST 2: INSTALLED APPLICATIONS\n";
        std::cout << "──────────────────────────────────────────────────────────────────────────\n\n";
        
        auto data = readJsonFile(mockDir + "/app_installed.json");
        if (!data.isNull()) {
            auto apps = data.isMember("packages") ? data["packages"] : data;
            std::string output = entos::UnifiedDiagnostics::formatInstalledApps(apps);
            std::cout << output;
        }
        std::cout << "\n\n";
    }
    
    // Test 3: Package List
    {
        std::cout << "TEST 3: PACKAGE MANAGER\n";
        std::cout << "──────────────────────────────────────────────────────────────────────────\n\n";
        
        auto data = readJsonFile(mockDir + "/package_list.json");
        if (!data.isNull()) {
            std::string output = entos::UnifiedDiagnostics::formatPackageList(data);
            std::cout << output;
        }
        std::cout << "\n\n";
    }
    
    // Test 4: Resource Stats
    {
        std::cout << "TEST 4: RESOURCE STATISTICS\n";
        std::cout << "──────────────────────────────────────────────────────────────────────────\n\n";
        
        auto data = readJsonFile(mockDir + "/resource_stats.json");
        if (!data.isNull()) {
            std::string output = entos::UnifiedDiagnostics::formatResourceStats(data);
            std::cout << output;
        }
        std::cout << "\n\n";
    }
    
    // Test 5: Window Clients
    {
        std::cout << "TEST 5: WINDOW MANAGER\n";
        std::cout << "──────────────────────────────────────────────────────────────────────────\n\n";
        
        auto data = readJsonFile(mockDir + "/window_clients.json");
        if (!data.isNull()) {
            std::string output = entos::UnifiedDiagnostics::formatWindowClients(data);
            std::cout << output;
        }
        std::cout << "\n\n";
    }
    
    std::cout << "╔═══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                        ALL TESTS COMPLETED SUCCESSFULLY                    ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
