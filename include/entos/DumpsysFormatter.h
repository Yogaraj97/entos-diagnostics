#pragma once

#include <string>
#include <vector>
#include <json/json.h>

namespace entos {

class DumpsysFormatter {
public:
    // Runtime Manager formatting (Loaded & Installed Apps)
    static std::string formatRuntimeManager(const Json::Value& data);
    
    // Package Manager formatting
    static std::string formatPackageManager(const Json::Value& data);
    
    // Resource Monitor formatting
    static std::string formatResourceMonitor(const Json::Value& data);
    
    // Window Manager formatting
    static std::string formatWindowManager(const Json::Value& data);
    
    // Main dumpsys output builder
    static std::string buildDumpsysOutput(const Json::Value& sections);

private:
    // Helper methods
    static std::string createSectionHeader(const std::string& title);
    static std::string createTableHeader(const std::vector<std::string>& columns);
    static std::string createTableSeparator(const std::vector<std::string>& columns);
    static std::string createTableRow(const std::vector<std::string>& values, 
                                      const std::vector<std::string>& columns);
    static std::string padColumn(const std::string& value, size_t width);
    static size_t calculateColumnWidth(const std::vector<std::string>& values);
};

}  // namespace entos
