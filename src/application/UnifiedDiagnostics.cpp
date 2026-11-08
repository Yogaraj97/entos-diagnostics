#include "entos/UnifiedDiagnostics.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace entos {

// TableBuilder Implementation
TableBuilder::TableBuilder(const std::vector<Column>& columns)
    : columns_(columns) {}

void TableBuilder::addRow(const TableRow& row) {
    rows_.push_back(row);
}

void TableBuilder::addSection(const std::string& title) {
    sections_.push_back(title);
}

void TableBuilder::addSummary(const std::string& text) {
    summaries_.push_back(text);
}

std::string TableBuilder::padValue(const std::string& value, std::size_t width, const std::string& alignment) const {
    if (value.length() >= width) {
        return value.substr(0, width);
    }
    
    std::size_t padding = width - value.length();
    std::string result = value;
    
    if (alignment == "right") {
        result = std::string(padding, ' ') + value;
    } else if (alignment == "center") {
        std::size_t left_pad = padding / 2;
        std::size_t right_pad = padding - left_pad;
        result = std::string(left_pad, ' ') + value + std::string(right_pad, ' ');
    } else {  // left
        result = value + std::string(padding, ' ');
    }
    
    return result;
}

std::string TableBuilder::createHeader() const {
    std::ostringstream oss;
    for (std::size_t i = 0; i < columns_.size(); ++i) {
        if (i > 0) oss << " | ";
        oss << padValue(columns_[i].header, columns_[i].width, "left");
    }
    return oss.str();
}

std::string TableBuilder::createSeparator() const {
    std::ostringstream oss;
    for (std::size_t i = 0; i < columns_.size(); ++i) {
        if (i > 0) oss << "-+-";
        oss << std::string(columns_[i].width, '-');
    }
    return oss.str();
}

std::string TableBuilder::formatRow(const TableRow& row) const {
    std::ostringstream oss;
    for (std::size_t i = 0; i < columns_.size() && i < row.values.size(); ++i) {
        if (i > 0) oss << " | ";
        oss << padValue(row.values[i], columns_[i].width, columns_[i].alignment);
    }
    return oss.str();
}

std::string TableBuilder::build() const {
    std::ostringstream oss;
    
    oss << createHeader() << "\n";
    oss << createSeparator() << "\n";
    
    for (const auto& row : rows_) {
        oss << formatRow(row) << "\n";
    }
    
    if (!summaries_.empty()) {
        oss << "\n";
        for (const auto& summary : summaries_) {
            oss << "    " << summary << "\n";
        }
    }
    
    return oss.str();
}

// UnifiedDiagnostics Helper Methods
std::string UnifiedDiagnostics::getString(const Json::Value& obj, const std::string& key, const std::string& fallback) {
    if (obj.isMember(key) && obj[key].isString()) {
        return obj[key].asString();
    }
    return fallback;
}

std::string UnifiedDiagnostics::getInt(const Json::Value& obj, const std::string& key, const std::string& fallback) {
    if (obj.isMember(key) && obj[key].isIntegral()) {
        return std::to_string(obj[key].asInt64());
    }
    return fallback;
}

std::string UnifiedDiagnostics::getDouble(const Json::Value& obj, const std::string& key, int precision, const std::string& fallback) {
    if (obj.isMember(key) && obj[key].isNumeric()) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << obj[key].asDouble();
        return oss.str();
    }
    return fallback;
}

std::string UnifiedDiagnostics::formatBytes(long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit_index = 0;
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    if (unit_index == 0) {
        oss << static_cast<long>(size) << " " << units[unit_index];
    } else {
        oss << std::fixed << std::setprecision(1) << size << " " << units[unit_index];
    }
    return oss.str();
}

std::string UnifiedDiagnostics::formatPercent(double value) {
    if (std::isnan(value) || std::isinf(value)) {
        return "0.0%";
    }
    double clamped = std::max(0.0, std::min(100.0, value));
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << clamped << "%";
    return oss.str();
}

// UnifiedDiagnostics Format Methods

std::string UnifiedDiagnostics::formatLoadedApps(const Json::Value& apps) {
    std::vector<Column> columns = {
        {"App ID", 15, "left"},
        {"Instance ID", 18, "left"},
        {"State", 12, "left"},
        {"PID", 8, "right"}
    };
    
    TableBuilder builder(columns);
    
    if (apps.isArray()) {
        for (const auto& app : apps) {
            TableRow row;
            row.values.push_back(getString(app, "appId"));
            row.values.push_back(getString(app, "instanceId"));
            row.values.push_back(getString(app, "state"));
            row.values.push_back(getInt(app, "pid"));
            builder.addRow(row);
        }
        builder.addSummary("Summary: " + std::to_string(apps.size()) + " app(s) loaded");
    }
    
    return builder.build();
}

std::string UnifiedDiagnostics::formatInstalledApps(const Json::Value& apps) {
    std::vector<Column> columns = {
        {"App ID", 15, "left"},
        {"Name", 20, "left"},
        {"Version", 15, "left"},
        {"Category", 15, "left"}
    };
    
    TableBuilder builder(columns);
    
    if (apps.isArray()) {
        for (const auto& app : apps) {
            TableRow row;
            row.values.push_back(getString(app, "appId"));
            row.values.push_back(getString(app, "name"));
            row.values.push_back(getString(app, "version"));
            row.values.push_back(getString(app, "category"));
            builder.addRow(row);
        }
        builder.addSummary("Summary: " + std::to_string(apps.size()) + " app(s) installed");
    }
    
    return builder.build();
}

std::string UnifiedDiagnostics::formatPackageList(const Json::Value& data) {
    std::ostringstream oss;
    
    // Cache status section
    if (data.isMember("cache")) {
        const auto& cache = data["cache"];
        oss << "Cache Status:\n";
        if (cache.isMember("used")) {
            oss << "    Used: " << formatBytes(cache["used"].asInt64()) << "\n";
        }
        if (cache.isMember("limit")) {
            oss << "    Limit: " << formatBytes(cache["limit"].asInt64()) << "\n";
        }
        oss << "\n";
    }
    
    // Package table
    std::vector<Column> columns = {
        {"Package Name", 18, "left"},
        {"Version", 15, "left"},
        {"Size", 12, "right"},
        {"Type", 8, "left"},
        {"Installed", 12, "center"}
    };
    
    TableBuilder builder(columns);
    
    if (data.isMember("packages") && data["packages"].isArray()) {
        const auto& packages = data["packages"];
        for (const auto& pkg : packages) {
            TableRow row;
            row.values.push_back(getString(pkg, "name"));
            row.values.push_back(getString(pkg, "version"));
            
            if (pkg.isMember("size")) {
                row.values.push_back(formatBytes(pkg["size"].asInt64()));
            } else {
                row.values.push_back("0 B");
            }
            
            row.values.push_back(getString(pkg, "type"));
            row.values.push_back(getString(pkg, "installed", "No"));
            builder.addRow(row);
        }
        builder.addSummary("Summary: " + std::to_string(packages.size()) + " package(s) in database");
    }
    
    oss << builder.build();
    return oss.str();
}

std::string UnifiedDiagnostics::formatResourceStats(const Json::Value& data) {
    std::ostringstream oss;
    
    // Total resources section
    if (data.isMember("totals")) {
        const auto& totals = data["totals"];
        oss << "Total Resources:\n";
        if (totals.isMember("cpu")) {
            oss << "    CPU: " << formatPercent(totals["cpu"].asDouble()) << "\n";
        }
        if (totals.isMember("memory")) {
            oss << "    Memory: " << formatBytes(totals["memory"].asInt64()) << "\n";
        }
        oss << "\n";
    }
    
    // Per-app stats
    std::vector<Column> columns = {
        {"App ID", 15, "left"},
        {"CPU (%)", 10, "right"},
        {"Memory (MB)", 12, "right"},
        {"Timestamp", 12, "right"}
    };
    
    TableBuilder builder(columns);
    
    if (data.isMember("apps") && data["apps"].isArray()) {
        const auto& apps = data["apps"];
        for (const auto& app : apps) {
            TableRow row;
            row.values.push_back(getString(app, "appId"));
            row.values.push_back(getDouble(app, "cpu", 1));
            
            if (app.isMember("memory")) {
                long mem_bytes = app["memory"].asInt64();
                row.values.push_back(std::to_string(mem_bytes / (1024 * 1024)));
            } else {
                row.values.push_back("0");
            }
            
            row.values.push_back(getInt(app, "timestamp"));
            builder.addRow(row);
        }
        builder.addSummary("Summary: " + std::to_string(apps.size()) + " app(s) monitored");
    }
    
    oss << builder.build();
    return oss.str();
}

std::string UnifiedDiagnostics::formatWindowClients(const Json::Value& data) {
    std::ostringstream oss;
    
    // Focused application
    if (data.isMember("focusedApp")) {
        oss << "Focused Application: " << getString(data, "focusedApp") << "\n\n";
    }
    
    // Window state table
    std::vector<Column> columns = {
        {"App ID", 15, "left"},
        {"Z-Order", 10, "right"},
        {"Focused", 10, "center"},
        {"Visible", 10, "center"}
    };
    
    TableBuilder builder(columns);
    
    if (data.isMember("clients") && data["clients"].isArray()) {
        const auto& clients = data["clients"];
        for (const auto& client : clients) {
            TableRow row;
            row.values.push_back(getString(client, "appId"));
            row.values.push_back(getInt(client, "zOrder"));
            row.values.push_back(client.isMember("focused") && client["focused"].asBool() ? "Yes" : "No");
            row.values.push_back(client.isMember("visible") && client["visible"].asBool() ? "Yes" : "No");
            builder.addRow(row);
        }
        builder.addSummary("Summary: " + std::to_string(clients.size()) + " window client(s)");
    }
    
    oss << builder.build();
    return oss.str();
}

std::string UnifiedDiagnostics::formatAppManagerDump(const Json::Value& data) {
    std::ostringstream oss;
    
    oss << "APP MANAGER DIAGNOSTICS\n";
    oss << "=======================\n\n";
    
    if (data.isMember("loadedApps")) {
        oss << "LOADED APPLICATIONS:\n";
        oss << "===================\n\n";
        oss << formatLoadedApps(data["loadedApps"]) << "\n";
    }
    
    if (data.isMember("installedApps")) {
        oss << "INSTALLED APPLICATIONS:\n";
        oss << "======================\n\n";
        oss << formatInstalledApps(data["installedApps"]) << "\n";
    }
    
    if (data.isMember("packages")) {
        oss << "PACKAGE MANAGER:\n";
        oss << "===============\n\n";
        oss << formatPackageList(data) << "\n";
    }
    
    if (data.isMember("resources")) {
        oss << "RESOURCE STATISTICS:\n";
        oss << "===================\n\n";
        oss << formatResourceStats(data["resources"]) << "\n";
    }
    
    if (data.isMember("windowClients")) {
        oss << "WINDOW MANAGER:\n";
        oss << "==============\n\n";
        oss << formatWindowClients(data["windowClients"]) << "\n";
    }
    
    return oss.str();
}

std::string UnifiedDiagnostics::formatSystemInfo(const std::string& manager, const Json::Value& data) {
    // Generic formatter for future managers
    std::ostringstream oss;
    oss << manager.substr(0, 1);
    for (char c : manager.substr(1)) {
        if (c == '_') continue;
        if (std::isupper(c)) oss << " ";
        oss << c;
    }
    
    std::string title = oss.str();
    std::transform(title.begin(), title.end(), title.begin(), ::toupper);
    
    oss.str("");
    oss << title << "\n";
    oss << std::string(title.length(), '=') << "\n\n";
    
    // Generic JSON to table (can be customized per manager)
    if (data.isArray()) {
        oss << "Items: " << data.size() << "\n";
    } else if (data.isObject()) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            oss << it.name() << ": " << it->asString() << "\n";
        }
    }
    
    return oss.str();
}

} // namespace entos
