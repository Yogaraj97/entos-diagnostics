#pragma once

#include <string>
#include <vector>
#include <json/json.h>
#include <map>
#include <optional>

namespace entos {

/**
 * UnifiedDiagnostics: Extensible diagnostic output formatter
 * 
 * Supports multiple managers (AppManager, WindowManager, PackageManager, etc.)
 * with consistent human-readable table output format.
 * 
 * Architecture:
 * - Provider-agnostic table rendering
 * - Column-based alignment
 * - Summary statistics
 * - Pluggable formatter implementations
 */

struct Column {
    std::string header;
    std::size_t width;
    std::string alignment = "left";  // left, center, right
};

struct TableRow {
    std::vector<std::string> values;
};

class TableBuilder {
public:
    TableBuilder(const std::vector<Column>& columns);
    
    void addRow(const TableRow& row);
    void addSection(const std::string& title);
    void addSummary(const std::string& text);
    
    std::string build() const;
    
private:
    std::vector<Column> columns_;
    std::vector<TableRow> rows_;
    std::vector<std::string> sections_;
    std::vector<std::string> summaries_;
    
    std::string createHeader() const;
    std::string createSeparator() const;
    std::string formatRow(const TableRow& row) const;
    std::string padValue(const std::string& value, std::size_t width, const std::string& alignment) const;
};

class UnifiedDiagnostics {
public:
    /**
     * Format AppManager loaded applications
     */
    static std::string formatLoadedApps(const Json::Value& apps);
    
    /**
     * Format AppManager installed applications
     */
    static std::string formatInstalledApps(const Json::Value& apps);
    
    /**
     * Format package manager information
     */
    static std::string formatPackageList(const Json::Value& packages);
    
    /**
     * Format resource statistics
     */
    static std::string formatResourceStats(const Json::Value& stats);
    
    /**
     * Format window manager clients
     */
    static std::string formatWindowClients(const Json::Value& clients);
    
    /**
     * Format complete AppManager diagnostic dump
     */
    static std::string formatAppManagerDump(const Json::Value& data);
    
    /**
     * Format system information (for extensibility to other managers)
     */
    static std::string formatSystemInfo(const std::string& manager, const Json::Value& data);
    
private:
    /**
     * Helper to extract string value from JSON with fallback
     */
    static std::string getString(const Json::Value& obj, const std::string& key, const std::string& fallback = "N/A");
    
    /**
     * Helper to extract integer value from JSON
     */
    static std::string getInt(const Json::Value& obj, const std::string& key, const std::string& fallback = "0");
    
    /**
     * Helper to extract double value from JSON with precision
     */
    static std::string getDouble(const Json::Value& obj, const std::string& key, int precision = 1, const std::string& fallback = "0.0");
    
    /**
     * Helper to format bytes as human-readable
     */
    static std::string formatBytes(long bytes);
    
    /**
     * Helper to format percentage with proper range checking
     */
    static std::string formatPercent(double value);
};

} // namespace entos
