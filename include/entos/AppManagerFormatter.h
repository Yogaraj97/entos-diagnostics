#pragma once

#include <json/json.h>
#include <string>
#include <vector>

namespace entos {

/**
 * Human-readable formatter for AppManager diagnostic data
 * Outputs table and simplified formats (not JSON)
 */
class AppManagerFormatter {
public:
    /**
     * Format loaded applications as a readable table
     * @param data JSON response from getLoadedApps
     * @return Formatted table string
     */
    static std::string formatLoadedApps(const Json::Value& data);

    /**
     * Format installed applications as a readable table
     * @param data JSON response from getInstalledApps
     * @return Formatted table string
     */
    static std::string formatInstalledApps(const Json::Value& data);

    /**
     * Format package list as a readable table
     * @param data JSON response from listPackages
     * @return Formatted table string
     */
    static std::string formatPackageList(const Json::Value& data);

    /**
     * Format resource statistics as a readable table
     * @param data JSON response from GetStats
     * @return Formatted table string
     */
    static std::string formatResourceStats(const Json::Value& data);

    /**
     * Format window clients as a readable table
     * @param data JSON response from getApps
     * @return Formatted table string
     */
    static std::string formatWindowClients(const Json::Value& data);

    /**
     * Format complete AppManager dump for dumpsys output
     * @param snapshot Complete diagnostic snapshot
     * @return Formatted text output
     */
    static std::string formatAppManagerDump(const Json::Value& snapshot);

private:
    static std::string createTableHeader(const std::vector<std::string>& columns);
    static std::string createTableRow(const std::vector<std::string>& values);
    static std::string createTableSeparator(const std::vector<std::string>& columns);
    static int getColumnWidth(const std::string& header);
};

} // namespace entos
