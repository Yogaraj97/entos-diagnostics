#include "entos/AppManagerFormatter.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace entos {

int AppManagerFormatter::getColumnWidth(const std::string& header) {
    return std::max(static_cast<int>(header.length()) + 2, 10);
}

std::string AppManagerFormatter::createTableSeparator(const std::vector<std::string>& columns) {
    std::stringstream ss;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) ss << "-+-";
        ss << std::string(getColumnWidth(columns[i]), '-');
    }
    ss << '\n';
    return ss.str();
}

std::string AppManagerFormatter::createTableHeader(const std::vector<std::string>& columns) {
    std::stringstream ss;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) ss << " | ";
        ss << std::left << std::setw(getColumnWidth(columns[i])) << columns[i];
    }
    ss << '\n';
    ss << createTableSeparator(columns);
    return ss.str();
}

std::string AppManagerFormatter::createTableRow(const std::vector<std::string>& values) {
    std::stringstream ss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) ss << " | ";
        int width = (i < 2) ? getColumnWidth("App ID") : getColumnWidth("Value");
        ss << std::left << std::setw(width) << values[i];
    }
    ss << '\n';
    return ss.str();
}

std::string AppManagerFormatter::formatLoadedApps(const Json::Value& data) {
    std::stringstream output;
    
    output << "\nLOADED APPLICATIONS:\n";
    output << "===================\n";
    
    if (!data.isMember("loaded_apps") || !data["loaded_apps"].isArray()) {
        output << "    No loaded applications found\n";
        return output.str();
    }
    
    const auto& apps = data["loaded_apps"];
    if (apps.size() == 0) {
        output << "    No applications currently loaded\n";
        return output.str();
    }
    
    // Table header
    std::vector<std::string> headers{"App ID", "Instance ID", "State", "PID"};
    output << createTableHeader(headers);
    
    // Table rows
    for (const auto& app : apps) {
        std::vector<std::string> row;
        row.push_back(app.get("appId", "--").asString());
        row.push_back(app.get("instanceId", "--").asString());
        row.push_back(app.get("state", "--").asString());
        
        if (app.isMember("pid")) {
            row.push_back(std::to_string(app["pid"].asInt()));
        } else {
            row.push_back("--");
        }
        
        output << createTableRow(row);
    }
    
    output << "\n    Summary: " << apps.size() << " app(s) loaded\n";
    return output.str();
}

std::string AppManagerFormatter::formatInstalledApps(const Json::Value& data) {
    std::stringstream output;
    
    output << "\nINSTALLED APPLICATIONS:\n";
    output << "======================\n";
    
    if (!data.isMember("installed_apps") || !data["installed_apps"].isArray()) {
        output << "    No installed applications found\n";
        return output.str();
    }
    
    const auto& apps = data["installed_apps"];
    if (apps.size() == 0) {
        output << "    No applications installed\n";
        return output.str();
    }
    
    // Table header
    std::vector<std::string> headers{"App ID", "Name", "Version", "Category"};
    output << createTableHeader(headers);
    
    // Table rows
    for (const auto& app : apps) {
        std::vector<std::string> row;
        row.push_back(app.get("appId", "--").asString());
        row.push_back(app.get("name", "--").asString());
        row.push_back(app.get("version", "--").asString());
        row.push_back(app.get("category", "--").asString());
        output << createTableRow(row);
    }
    
    output << "\n    Summary: " << apps.size() << " app(s) installed\n";
    return output.str();
}

std::string AppManagerFormatter::formatPackageList(const Json::Value& data) {
    std::stringstream output;
    
    output << "\nPACKAGE MANAGER:\n";
    output << "===============\n";
    
    // Package cache info
    if (data.isMember("cache")) {
        const auto& cache = data["cache"];
        output << "    Cache Status:\n";
        if (cache.isMember("used_bytes")) {
            long usedMB = cache["used_bytes"].asInt64() / (1024 * 1024);
            output << "        Used: " << usedMB << " MB\n";
        }
        if (cache.isMember("limit_bytes")) {
            long limitMB = cache["limit_bytes"].asInt64() / (1024 * 1024);
            output << "        Limit: " << limitMB << " MB\n";
        }
    }
    
    // Packages list
    if (!data.isMember("packages") || !data["packages"].isArray()) {
        output << "\n    No packages found\n";
        return output.str();
    }
    
    const auto& packages = data["packages"];
    if (packages.size() == 0) {
        output << "\n    No packages in database\n";
        return output.str();
    }
    
    output << "\n    Packages:\n";
    std::vector<std::string> headers{"Package Name", "Version", "Size", "Type", "Installed"};
    output << createTableHeader(headers);
    
    for (const auto& pkg : packages) {
        std::vector<std::string> row;
        row.push_back(pkg.get("name", "--").asString());
        row.push_back(pkg.get("version", "--").asString());
        row.push_back(pkg.get("size", "--").asString());
        row.push_back(pkg.get("type", "--").asString());
        row.push_back(pkg.get("installed", false).asBool() ? "Yes" : "No");
        output << createTableRow(row);
    }
    
    output << "\n    Summary: " << packages.size() << " package(s) in database\n";
    return output.str();
}

std::string AppManagerFormatter::formatResourceStats(const Json::Value& data) {
    std::stringstream output;
    
    output << "\nRESOURCE MONITOR:\n";
    output << "===============\n";
    
    // Totals
    if (data.isMember("total_cpu") || data.isMember("total_memory_bytes")) {
        output << "    Total Resources:\n";
        if (data.isMember("total_cpu")) {
            output << "        CPU: " << std::fixed << std::setprecision(1) 
                   << data["total_cpu"].asFloat() << "%\n";
        }
        if (data.isMember("total_memory_bytes")) {
            long memMB = data["total_memory_bytes"].asInt64() / (1024 * 1024);
            output << "        Memory: " << memMB << " MB\n";
        }
    }
    
    // Per-app stats
    if (!data.isMember("stats") || !data["stats"].isArray()) {
        output << "\n    No per-app statistics available\n";
        return output.str();
    }
    
    const auto& stats = data["stats"];
    if (stats.size() == 0) {
        output << "\n    No running applications\n";
        return output.str();
    }
    
    output << "\n    Per-Application Stats:\n";
    std::vector<std::string> headers{"App ID", "CPU (%)", "Memory (MB)", "Timestamp"};
    output << createTableHeader(headers);
    
    for (const auto& stat : stats) {
        std::vector<std::string> row;
        row.push_back(stat.get("appId", "--").asString());
        
        if (stat.isMember("cpu")) {
            std::stringstream cpu;
            cpu << std::fixed << std::setprecision(1) << stat["cpu"].asFloat();
            row.push_back(cpu.str());
        } else {
            row.push_back("--");
        }
        
        if (stat.isMember("memory_bytes")) {
            long memMB = stat["memory_bytes"].asInt64() / (1024 * 1024);
            row.push_back(std::to_string(memMB));
        } else {
            row.push_back("--");
        }
        
        if (stat.isMember("timestamp")) {
            row.push_back(std::to_string(stat["timestamp"].asInt64()));
        } else {
            row.push_back("--");
        }
        
        output << createTableRow(row);
    }
    
    output << "\n    Summary: " << stats.size() << " app(s) monitored\n";
    return output.str();
}

std::string AppManagerFormatter::formatWindowClients(const Json::Value& data) {
    std::stringstream output;
    
    output << "\nWINDOW MANAGER:\n";
    output << "===============\n";
    
    // Focused app
    if (data.isMember("focused_app_id")) {
        output << "    Focused Application: " << data["focused_app_id"].asString() << "\n";
    }
    
    // Window clients
    if (!data.isMember("apps") || !data["apps"].isArray()) {
        output << "\n    No window clients found\n";
        return output.str();
    }
    
    const auto& apps = data["apps"];
    if (apps.size() == 0) {
        output << "\n    No applications in window manager\n";
        return output.str();
    }
    
    output << "\n    Window Clients:\n";
    std::vector<std::string> headers{"App ID", "Z-Order", "Focused", "Visible"};
    output << createTableHeader(headers);
    
    for (const auto& app : apps) {
        std::vector<std::string> row;
        row.push_back(app.get("appId", "--").asString());
        
        if (app.isMember("zOrder")) {
            row.push_back(std::to_string(app["zOrder"].asInt()));
        } else {
            row.push_back("--");
        }
        
        row.push_back(app.get("focused", false).asBool() ? "Yes" : "No");
        row.push_back(app.get("visible", false).asBool() ? "Yes" : "No");
        
        output << createTableRow(row);
    }
    
    output << "\n    Summary: " << apps.size() << " window client(s)\n";
    return output.str();
}

std::string AppManagerFormatter::formatAppManagerDump(const Json::Value& snapshot) {
    std::stringstream output;
    
    output << "APP MANAGER DIAGNOSTICS\n";
    output << "=======================\n";
    
    // Runtime section
    if (snapshot.isMember("sections")) {
        const auto& sections = snapshot["sections"];
        for (const auto& section : sections) {
            if (section.get("id", "").asString() == "runtime") {
                output << "\nRUNTIME SECTION:\n";
                output << "    Status: " << section.get("status", "unknown").asString() << "\n\n";
                
                // Parse and format loaded apps
                if (section.isMember("app.loaded.summary")) {
                    const auto& appData = section["app.loaded.summary"];
                    if (appData.isObject() && appData.isMember("loaded_apps")) {
                        output << formatLoadedApps(appData);
                    }
                }
                
                // Parse and format installed apps
                if (section.isMember("app.installed.summary")) {
                    const auto& appData = section["app.installed.summary"];
                    if (appData.isObject() && appData.isMember("installed_apps")) {
                        output << formatInstalledApps(appData);
                    }
                }
            }
            
            if (section.get("id", "").asString() == "package") {
                output << "\nPACKAGE SECTION:\n";
                output << "    Status: " << section.get("status", "unknown").asString() << "\n";
                
                if (section.isMember("package.list")) {
                    const auto& pkgData = section["package.list"];
                    if (pkgData.isObject() && pkgData.isMember("packages")) {
                        output << formatPackageList(pkgData);
                    }
                }
            }
            
            if (section.get("id", "").asString() == "resource") {
                output << "\nRESOURCE SECTION:\n";
                output << "    Status: " << section.get("status", "unknown").asString() << "\n";
                
                if (section.isMember("resource.stats.summary")) {
                    const auto& statsData = section["resource.stats.summary"];
                    if (statsData.isObject() && statsData.isMember("stats")) {
                        output << formatResourceStats(statsData);
                    }
                }
            }
            
            if (section.get("id", "").asString() == "window") {
                output << "\nWINDOW SECTION:\n";
                output << "    Status: " << section.get("status", "unknown").asString() << "\n";
                
                if (section.isMember("window.apps.summary")) {
                    const auto& windowData = section["window.apps.summary"];
                    if (windowData.isObject() && windowData.isMember("apps")) {
                        output << formatWindowClients(windowData);
                    }
                }
            }
        }
    }
    
    return output.str();
}

} // namespace entos
