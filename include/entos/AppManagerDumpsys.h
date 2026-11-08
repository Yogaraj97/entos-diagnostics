/**
 * AppManager Dumpsys Handler
 * 
 * Formats AppManager diagnostic output in human-readable table format
 * Usage: entos-diagnostics appmanager [options]
 * 
 * This is integrated with the main entos-diagnostics dumpsys system
 * to provide focused AppManager state snapshots with readable output.
 */

#pragma once

#include <json/json.h>
#include <string>
#include <vector>
#include "entos/RpcClient.h"

namespace entos {

/**
 * AppManager-specific diagnostic handler
 * Queries AppManager Thunder plugins and formats output
 * 
 * Architecture:
 * - RpcClient: HTTP/JSON-RPC communication with Thunder
 * - Query methods: getLoadedApps(), getInstalledApps(), etc.
 * - Formatting: Uses UnifiedDiagnostics for consistent table output
 * - Extensibility: Pattern can be replicated for other managers
 */
class AppManagerDumpsys {
public:
    explicit AppManagerDumpsys(RpcClient& rpc);
    
    /**
     * Query AppManager for loaded applications
     * @param timeoutMs Request timeout in milliseconds
     * @return Formatted table output
     */
    std::string getLoadedApps(long timeoutMs = 3000);
    
    /**
     * Query AppManager for installed applications
     * @param timeoutMs Request timeout in milliseconds
     * @return Formatted table output
     */
    std::string getInstalledApps(long timeoutMs = 3000);
    
    /**
     * Query AppManager for package inventory
     * @param timeoutMs Request timeout in milliseconds
     * @return Formatted table output
     */
    std::string getPackages(long timeoutMs = 3000);
    
    /**
     * Query AppManager for resource statistics
     * @param timeoutMs Request timeout in milliseconds
     * @return Formatted table output
     */
    std::string getResourceStats(long timeoutMs = 3000);
    
    /**
     * Query AppManager for window manager state
     * @param timeoutMs Request timeout in milliseconds
     * @return Formatted table output
     */
    std::string getWindowState(long timeoutMs = 3000);
    
    /**
     * Get complete AppManager dumpsys output
     * @param sections Which sections to include (empty = all)
     * @param timeoutMs Request timeout in milliseconds
     * @return Complete formatted diagnostic output
     */
    std::string dumpAll(const std::vector<std::string>& sections = {}, long timeoutMs = 3000);
    
private:
    RpcClient& rpc_;
    
    /**
     * Helper to query Thunder plugin with fallback callsigns
     */
    RpcReply queryThunder(const std::vector<std::string>& callsigns, 
                         const std::string& method,
                         const Json::Value& params,
                         long timeoutMs);
    
    /**
     * Helper to construct JSON-RPC params
     */
    static Json::Value makeParams();
};

} // namespace entos

