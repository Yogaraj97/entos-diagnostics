#include "entos/DumpsysFormatter.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace entos {

std::string DumpsysFormatter::padColumn(const std::string& value, size_t width) {
    if (value.length() >= width) {
        return value.substr(0, width);
    }
    return value + std::string(width - value.length(), ' ');
}

size_t DumpsysFormatter::calculateColumnWidth(const std::vector<std::string>& values) {
    size_t maxWidth = 0;
    for (const auto& value : values) {
        maxWidth = std::max(maxWidth, value.length());
    }
    return std::max(maxWidth, static_cast<size_t>(15));  // minimum width
}

std::string DumpsysFormatter::createSectionHeader(const std::string& title) {
    std::ostringstream oss;
    oss << "\n" << title << ":\n";
    oss << std::string(title.length() + 1, ' ') << "\n";
    return oss.str();
}

std::string DumpsysFormatter::createTableHeader(const std::vector<std::string>& columns) {
    std::ostringstream oss;
    
    // Calculate column widths
    std::vector<size_t> widths;
    for (const auto& col : columns) {
        widths.push_back(std::max(col.length() + 1, static_cast<size_t>(20)));
    }
    
    // Print headers
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) oss << "| ";
        oss << padColumn(columns[i], widths[i]);
    }
    oss << "\n";
    
    // Print separator
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) oss << "+-";
        oss << std::string(widths[i], '-');
    }
    oss << "\n";
    
    return oss.str();
}

std::string DumpsysFormatter::createTableSeparator(const std::vector<std::string>& columns) {
    std::ostringstream oss;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) oss << "+-";
        size_t width = std::max(columns[i].length() + 1, static_cast<size_t>(20));
        oss << std::string(width, '-');
    }
    oss << "\n";
    return oss.str();
}

std::string DumpsysFormatter::createTableRow(const std::vector<std::string>& values,
                                             const std::vector<std::string>& columns) {
    std::ostringstream oss;
    
    // Calculate column widths
    std::vector<size_t> widths;
    for (const auto& col : columns) {
        widths.push_back(std::max(col.length() + 1, static_cast<size_t>(20)));
    }
    
    // Print row values
    for (size_t i = 0; i < values.size() && i < columns.size(); ++i) {
        if (i > 0) oss << "| ";
        oss << padColumn(values[i], widths[i]);
    }
    oss << "\n";
    
    return oss.str();
}

std::string DumpsysFormatter::formatRuntimeManager(const Json::Value& data) {
    std::ostringstream output;
    
    output << createSectionHeader("RUNTIME MANAGER");
    
    // GUIAppManagerV2 section
    output << "    GUIAppManagerV2:\n";
    
    std::vector<std::string> columns = {
        "App", "Version", "Runtime", "Flags", "State", "Changed", "AS Token"
    };
    
    output << "        ";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) output << "| ";
        output << padColumn(columns[i], 25);
    }
    output << "\n";
    
    output << "        ";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) output << "+-";
        output << std::string(25, '-');
    }
    output << "\n";
    
    // Process loaded apps
    if (data.isMember("apps") && data["apps"].isArray()) {
        for (const auto& app : data["apps"]) {
            std::vector<std::string> row = {
                app.get("appId", "--").asString(),
                app.get("version", "--").asString(),
                app.get("runtime", "--").asString(),
                app.get("flags", "--").asString(),
                app.get("state", "--").asString(),
                app.get("changed", "--").asString(),
                app.get("asToken", "--").asString().substr(0, 8) + "..."
            };
            
            output << "        ";
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) output << "| ";
                output << padColumn(row[i], 25);
            }
            output << "\n";
        }
    }
    
    output << "\n    DaemonAppManager:\n";
    output << "        App                           | Version       | Runtime               | Flags | State       | Changed   | AS Token\n";
    output << "        ----------------------------- | ------------- | --------------------- | ----- | ----------- | --------- | ------------------------------------\n";
    output << "        (No daemon apps currently loaded)\n";
    
    return output.str();
}

std::string DumpsysFormatter::formatPackageManager(const Json::Value& data) {
    std::ostringstream output;
    
    output << createSectionHeader("PACKAGE MANAGER");
    
    // Installer section
    output << "    Installer:\n";
    output << "        Download Threads: " << data.get("downloadThreads", "2").asString() << "\n";
    output << "        Requests in Progress: " << data.get("requestsInProgress", "None").asString() << "\n";
    output << "        Requests Pending: " << data.get("requestsPending", "None").asString() << "\n";
    
    // Package Database section
    output << "    Package Database:\n";
    output << "        Compatible Platform Ids: " << data.get("platformId", "N/A").asString() << "\n";
    output << "        Storage Dir: " << data.get("storageDir", "/media/apps/sky/packages").asString() << "\n";
    output << "        RALF Packages Supported: true\n";
    output << "        Package Mount Mode: auto\n";
    output << "        Packages:\n";
    
    // Packages table
    output << "            App                                               | Version          | UserID\n";
    output << "            ------------------------------------------------- | ---------------- | --------------\n";
    
    if (data.isMember("packages") && data["packages"].isArray()) {
        for (const auto& pkg : data["packages"]) {
            std::string appName = pkg.get("app", "--").asString();
            std::string version = pkg.get("version", "--").asString();
            std::string userId = pkg.get("userId", "--").asString();
            
            // Pad app name to 51 characters
            appName = padColumn(appName, 51);
            version = padColumn(version, 18);
            
            output << "            " << appName << "| " << version << "| " << userId << "\n";
        }
    }
    
    // Package Cache section
    output << "    Package Cache:\n";
    output << "        Cache Size: " << data.get("cacheSize", "64 MB").asString() << "\n";
    output << "        Cache Limit: " << data.get("cacheLimit", "100 MB").asString() << "\n";
    output << "        Entries:\n";
    
    output << "            App                           | Version       | Size         | Last Used | Format  | Path\n";
    output << "            ----------------------------- | ------------- | ------------ | --------- | ------- | ---------------------------------\n";
    
    if (data.isMember("cache") && data["cache"].isArray()) {
        for (const auto& entry : data["cache"]) {
            std::string app = padColumn(entry.get("app", "--").asString(), 29);
            std::string version = padColumn(entry.get("version", "--").asString(), 13);
            std::string size = padColumn(entry.get("size", "--").asString(), 12);
            std::string lastUsed = padColumn(entry.get("lastUsed", "--").asString(), 9);
            std::string format = padColumn(entry.get("format", "--").asString(), 7);
            std::string path = entry.get("path", "--").asString();
            
            output << "            " << app << "| " << version << "| " << size << "| " 
                   << lastUsed << "| " << format << "| " << path << "\n";
        }
    }
    
    return output.str();
}

std::string DumpsysFormatter::formatResourceMonitor(const Json::Value& data) {
    std::ostringstream output;
    
    output << createSectionHeader("RESOURCE MONITOR");
    
    if (data.isMember("stats")) {
        const auto& stats = data["stats"];
        
        output << "    System Statistics:\n";
        output << "        Total CPU: " << stats.get("totalCpu", "N/A").asString() << "%\n";
        output << "        Total Memory: " << stats.get("totalMemory", "N/A").asString() << "\n";
        output << "        Used Memory: " << stats.get("usedMemory", "N/A").asString() << "\n";
        output << "        Free Memory: " << stats.get("freeMemory", "N/A").asString() << "\n";
        
        // Process list
        if (stats.isMember("processes") && stats["processes"].isArray()) {
            output << "\n    Process Information:\n";
            output << "        PID    | Name                          | CPU     | Memory\n";
            output << "        ------ | ----------------------------- | ------- | --------\n";
            
            for (const auto& proc : stats["processes"]) {
                std::string pid = padColumn(std::to_string(proc.get("pid", 0).asInt()), 6);
                std::string name = padColumn(proc.get("name", "--").asString(), 29);
                std::string cpu = padColumn(proc.get("cpu", "--").asString(), 7);
                std::string mem = proc.get("memory", "--").asString();
                
                output << "        " << pid << "| " << name << "| " << cpu << "| " << mem << "\n";
            }
        }
    }
    
    return output.str();
}

std::string DumpsysFormatter::formatWindowManager(const Json::Value& data) {
    std::ostringstream output;
    
    output << createSectionHeader("WINDOW MANAGER");
    
    output << "    Window Clients:\n";
    output << "        App                           | State       | Layer | Z-Order\n";
    output << "        ----------------------------- | ----------- | ----- | -------\n";
    
    if (data.isMember("clients") && data["clients"].isArray()) {
        for (const auto& client : data["clients"]) {
            std::string app = padColumn(client.get("app", "--").asString(), 29);
            std::string state = padColumn(client.get("state", "--").asString(), 11);
            std::string layer = padColumn(client.get("layer", "--").asString(), 5);
            std::string zorder = client.get("zorder", "--").asString();
            
            output << "        " << app << "| " << state << "| " << layer << "| " << zorder << "\n";
        }
    }
    
    if (data.isMember("focusedApp")) {
        output << "\n    Focused Application: " << data["focusedApp"].asString() << "\n";
    }
    
    return output.str();
}

std::string DumpsysFormatter::buildDumpsysOutput(const Json::Value& sections) {
    std::ostringstream output;
    
    output << "===========================================\n";
    output << "       ENTOS-DIAGNOSTICS DUMPSYS OUTPUT\n";
    output << "===========================================\n";
    
    if (sections.isMember("runtime") && sections["runtime"].isMember("data")) {
        output << formatRuntimeManager(sections["runtime"]["data"]);
    }
    
    if (sections.isMember("package") && sections["package"].isMember("data")) {
        output << formatPackageManager(sections["package"]["data"]);
    }
    
    if (sections.isMember("resource") && sections["resource"].isMember("data")) {
        output << formatResourceMonitor(sections["resource"]["data"]);
    }
    
    if (sections.isMember("window") && sections["window"].isMember("data")) {
        output << formatWindowManager(sections["window"]["data"]);
    }
    
    output << "\n";
    
    return output.str();
}

}  // namespace entos
