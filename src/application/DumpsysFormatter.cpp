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
        "App", "Instance ID", "State", "PID"
    };
    
    output << "        ";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) output << " | ";
        output << padColumn(columns[i], 20);
    }
    output << "\n";
    
    output << "        ";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) output << "-+-";
        output << std::string(20, '-');
    }
    output << "\n";
    
    // Process loaded apps - ONLY use loaded_apps (not installed_apps which has different fields)
    Json::Value apps_array;
    if (data.isMember("loaded_apps") && data["loaded_apps"].isArray()) {
        apps_array = data["loaded_apps"];
    } else if (data.isMember("apps") && data["apps"].isArray()) {
        apps_array = data["apps"];
    }
    
    if (!apps_array.empty() && apps_array.isArray()) {
        for (const auto& app : apps_array) {
            std::vector<std::string> row;
            
            // App ID
            if (app.isMember("appId")) {
                row.push_back(app["appId"].asString());
            } else {
                row.push_back("--");
            }
            
            // Instance ID
            if (app.isMember("instanceId")) {
                row.push_back(app["instanceId"].asString());
            } else {
                row.push_back("--");
            }
            
            // State
            if (app.isMember("state")) {
                row.push_back(app["state"].asString());
            } else {
                row.push_back("--");
            }
            
            // PID
            if (app.isMember("pid")) {
                row.push_back(std::to_string(app["pid"].asInt()));
            } else {
                row.push_back("--");
            }
            
            output << "        ";
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) output << " | ";
                output << padColumn(row[i], 20);
            }
            output << "\n";
        }
    }
    
    output << "\n    Summary:\n";
    if (apps_array.isArray()) {
        output << "        Loaded Apps: " << apps_array.size() << "\n";
    } else {
        output << "        Loaded Apps: 0\n";
    }
    
    return output.str();
}

std::string DumpsysFormatter::formatPackageManager(const Json::Value& data) {
    std::ostringstream output;
    
    output << createSectionHeader("PACKAGE MANAGER");
    
    // Check if this is listPackages data (array of packages)
    if (data.isMember("packages") && data["packages"].isArray()) {
        const auto& packages = data["packages"];
        
        // For listPackages API response
        output << "    Installed Packages:\n";
        output << "        App                          | Version              | State       | Size (KB) | Package Type\n";
        output << "        ---------------------------- | -------------------- | ----------- | --------- | -----------------------\n";
        
        if (packages.size() == 0) {
            output << "        (No packages found)\n";
        } else {
            for (const auto& pkg : packages) {
                std::string packageId = pkg.get("packageId", "unknown").asString();
                std::string version = pkg.get("version", "unknown").asString();
                std::string state = pkg.get("state", "unknown").asString();
                int sizeKb = pkg.get("sizeKb", 0).asInt();
                std::string packageType = pkg.get("packageType", "unknown").asString();
                
                output << "        " << std::left << std::setw(29) << packageId
                       << std::left << std::setw(21) << version
                       << std::left << std::setw(12) << state
                       << std::right << std::setw(10) << sizeKb
                       << std::left << std::setw(1) << " "
                       << packageType << "\n";
            }
        }
    } else {
        // For other package manager responses
        output << "    Installer:\n";
        int downloadThreads = data.get("downloadThreads", 0).asInt();
        output << "        Download Threads: " << downloadThreads << "\n";
        
        if (data.isMember("inProgress") && data["inProgress"].isArray() && data["inProgress"].size() > 0) {
            output << "        Requests in Progress:\n";
            for (const auto& request : data["inProgress"]) {
                std::string id = request.get("id", "unknown").asString();
                std::string url = request.get("url", "").asString();
                int rateLimit = request.get("rateLimit", 0).asInt();
                output << "            " << id << " - " << url << " (Rate: " << (rateLimit > 0 ? std::to_string(rateLimit) + " bps" : "none") << ")\n";
            }
        } else {
            output << "        Requests in Progress: None\n";
        }
        
        if (data.isMember("pending") && data["pending"].isArray() && data["pending"].size() > 0) {
            output << "        Requests Pending:\n";
            for (const auto& request : data["pending"]) {
                std::string id = request.get("id", "unknown").asString();
                std::string url = request.get("url", "").asString();
                int rateLimit = request.get("rateLimit", 0).asInt();
                output << "            " << id << " - " << url << " (Rate: " << (rateLimit > 0 ? std::to_string(rateLimit) + " bps" : "none") << ")\n";
            }
        } else {
            output << "        Requests Pending: None\n";
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
