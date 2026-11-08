#include "entos/AppManagerDumpsys.h"
#include "entos/UnifiedDiagnostics.h"
#include <stdexcept>
#include <sstream>

namespace entos {

AppManagerDumpsys::AppManagerDumpsys(RpcClient& rpc)
    : rpc_(rpc) {}

Json::Value AppManagerDumpsys::makeParams() {
    Json::Value params;
    return params;  // Empty params for most queries
}

RpcReply AppManagerDumpsys::queryThunder(const std::vector<std::string>& callsigns,
                                         const std::string& method,
                                         const Json::Value& params,
                                         long timeoutMs) {
    RpcReply reply;
    
    // Try each callsign until one works (for fallback support)
    for (const auto& callsign : callsigns) {
        std::string fullMethod = callsign + ".1." + method;
        reply = rpc_.call(fullMethod, params, timeoutMs);
        
        // -32601 = Method not found (try next callsign)
        if (reply.code != -32601) {
            break;
        }
    }
    
    return reply;
}

std::string AppManagerDumpsys::getLoadedApps(long timeoutMs) {
    // Thunder plugin: org.rdk.AppManager.getLoadedApps
    std::vector<std::string> callsigns = {
        "org.rdk.AppManager",
        "com.comcast.appmanager"  // fallback
    };
    
    auto reply = queryThunder(callsigns, "getLoadedApps", makeParams(), timeoutMs);
    
    if (reply.status != "ok") {
        return "ERROR: Failed to query loaded apps: " + reply.error + "\n";
    }
    
    // Expected response format: { result: { apps: [...] } }
    Json::Value apps = Json::Value(Json::arrayValue);
    if (reply.data.isMember("apps")) {
        apps = reply.data["apps"];
    } else if (reply.data.isArray()) {
        apps = reply.data;
    }
    
    return UnifiedDiagnostics::formatLoadedApps(apps);
}

std::string AppManagerDumpsys::getInstalledApps(long timeoutMs) {
    // Thunder plugin: org.rdk.AppPackageManager.listPackages
    std::vector<std::string> callsigns = {
        "org.rdk.AppPackageManager",
        "org.rdk.PackageManager",  // fallback
        "com.comcast.packagemanager"
    };
    
    auto reply = queryThunder(callsigns, "listPackages", makeParams(), timeoutMs);
    
    if (reply.status != "ok") {
        return "ERROR: Failed to query installed apps: " + reply.error + "\n";
    }
    
    // Expected response format: { result: { packages: [...] } }
    Json::Value apps = Json::Value(Json::arrayValue);
    if (reply.data.isMember("packages")) {
        apps = reply.data["packages"];
    } else if (reply.data.isArray()) {
        apps = reply.data;
    }
    
    return UnifiedDiagnostics::formatInstalledApps(apps);
}

std::string AppManagerDumpsys::getPackages(long timeoutMs) {
    // Thunder plugin: org.rdk.AppPackageManager.getPackageInfo
    std::vector<std::string> callsigns = {
        "org.rdk.AppPackageManager",
        "org.rdk.PackageManager",
        "com.comcast.packagemanager"
    };
    
    auto reply = queryThunder(callsigns, "getPackageInfo", makeParams(), timeoutMs);
    
    if (reply.status != "ok") {
        return "ERROR: Failed to query packages: " + reply.error + "\n";
    }
    
    // Expected response format: { cache: {...}, packages: [...] }
    // Use reply.data directly for formatPackageList
    return UnifiedDiagnostics::formatPackageList(reply.data);
}

std::string AppManagerDumpsys::getResourceStats(long timeoutMs) {
    // Thunder plugin: org.rdk.AppManager.getResourceStats
    std::vector<std::string> callsigns = {
        "org.rdk.AppManager",
        "com.comcast.appmanager"
    };
    
    auto reply = queryThunder(callsigns, "getResourceStats", makeParams(), timeoutMs);
    
    if (reply.status != "ok") {
        return "ERROR: Failed to query resource stats: " + reply.error + "\n";
    }
    
    // Expected response format: { totals: {...}, apps: [...] }
    return UnifiedDiagnostics::formatResourceStats(reply.data);
}

std::string AppManagerDumpsys::getWindowState(long timeoutMs) {
    // Thunder plugin: org.rdk.WindowManager.getClients
    std::vector<std::string> callsigns = {
        "org.rdk.WindowManager",
        "com.comcast.windowmanager"
    };
    
    auto reply = queryThunder(callsigns, "getClients", makeParams(), timeoutMs);
    
    if (reply.status != "ok") {
        return "ERROR: Failed to query window state: " + reply.error + "\n";
    }
    
    // Expected response format: { focusedApp: "...", clients: [...] }
    return UnifiedDiagnostics::formatWindowClients(reply.data);
}

std::string AppManagerDumpsys::dumpAll(const std::vector<std::string>& sections, long timeoutMs) {
    std::ostringstream output;
    
    // If no sections specified, dump all
    bool doLoaded = sections.empty() || 
                   std::find(sections.begin(), sections.end(), "loaded") != sections.end();
    bool doInstalled = sections.empty() || 
                      std::find(sections.begin(), sections.end(), "installed") != sections.end();
    bool doPackages = sections.empty() || 
                     std::find(sections.begin(), sections.end(), "packages") != sections.end();
    bool doResources = sections.empty() || 
                      std::find(sections.begin(), sections.end(), "resources") != sections.end();
    bool doWindow = sections.empty() || 
                   std::find(sections.begin(), sections.end(), "window") != sections.end();
    
    output << "APP MANAGER DIAGNOSTICS\n";
    output << "=======================\n\n";
    
    if (doLoaded) {
        output << "LOADED APPLICATIONS:\n";
        output << "===================\n\n";
        output << getLoadedApps(timeoutMs) << "\n";
    }
    
    if (doInstalled) {
        output << "INSTALLED APPLICATIONS:\n";
        output << "======================\n\n";
        output << getInstalledApps(timeoutMs) << "\n";
    }
    
    if (doPackages) {
        output << "PACKAGE MANAGER:\n";
        output << "===============\n\n";
        output << getPackages(timeoutMs) << "\n";
    }
    
    if (doResources) {
        output << "RESOURCE STATISTICS:\n";
        output << "===================\n\n";
        output << getResourceStats(timeoutMs) << "\n";
    }
    
    if (doWindow) {
        output << "WINDOW MANAGER:\n";
        output << "==============\n\n";
        output << getWindowState(timeoutMs) << "\n";
    }
    
    return output.str();
}

} // namespace entos
