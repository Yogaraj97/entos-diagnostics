#include "entos/Snapshot.h"
#include "entos/DumpsysFormatter.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <curl/curl.h>
#include <iomanip>
#include <memory>
#include <sstream>

namespace entos {
std::string timestamp()
{
    const auto now = std::time(nullptr);
    std::tm utc{};
    gmtime_r(&now, &utc);
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

Json::Value newSection(const SectionContract& contract)
{
    Json::Value section(Json::objectValue);
    section["id"] = contract.id;
    section["title"] = contract.title;
    section["collector"] = contract.id;
    section["owner"] = contract.owner;
    section["collectedAt"] = timestamp();
    section["status"] = "unsupported";
    section["data"] = Json::Value(Json::objectValue);
    section["coverage"] = Json::Value(Json::objectValue);
    section["errors"] = Json::Value(Json::arrayValue);
    for (const auto& name : contract.fields) {
        section["coverage"][name]["status"] = "unsupported";
        section["coverage"][name]["reason"] = "Requires a verified read-only provider from " + contract.owner;
        section["coverage"][name]["owner"] = contract.owner;
    }
    return section;
}

void recordField(Json::Value& section, const std::string& name, const Json::Value& value,
                 const std::string& source)
{
    section["data"][name] = value;
    section["coverage"][name]["status"] = "available";
    section["coverage"][name]["source"] = source;
    section["coverage"][name]["collectedAt"] = timestamp();
    section["coverage"][name].removeMember("reason");
}

void completeSection(Json::Value& section)
{
    bool available = false;
    bool missing = false;
    for (const auto& name : section["coverage"].getMemberNames()) {
        if (section["coverage"][name]["status"] == "available") available = true;
        else missing = true;
    }
    if (available) section["status"] = missing || !section["errors"].empty() ? "partial" : "ok";
}

Json::Value sanitize(const Json::Value& value, const std::vector<std::string>& secrets)
{
    if (value.isObject()) {
        Json::Value result(Json::objectValue);
        for (const auto& name : value.getMemberNames()) {
            auto lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
            const bool sensitive = lower.find("token") != std::string::npos
                || lower.find("password") != std::string::npos || lower.find("secret") != std::string::npos
                || lower.find("authorization") != std::string::npos || lower.find("intent") != std::string::npos
                || lower.find("sessionid") != std::string::npos || lower == "deviceid" || lower == "mac"
                || lower == "credentials" || lower == "keycontents";
            const auto safeName = sanitize(Json::Value(name), secrets).asString();
            result[safeName] = sensitive ? Json::Value("[REDACTED]") : sanitize(value[name], secrets);
        }
        return result;
    }
    if (value.isArray()) {
        Json::Value result(Json::arrayValue);
        for (const auto& entry : value) result.append(sanitize(entry, secrets));
        return result;
    }
    if (value.isString()) {
        auto text = value.asString();
        for (const auto& secret : secrets) {
            if (secret.empty()) continue;
            std::size_t position = 0;
            while ((position = text.find(secret, position)) != std::string::npos) {
                text.replace(position, secret.size(), "[REDACTED]");
                position += 10;
            }
        }
        auto lower = text;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        if (lower.find("bearer ") != std::string::npos || lower.find("token=") != std::string::npos
            || lower.find("password=") != std::string::npos || lower.find("secret=") != std::string::npos)
            return "[REDACTED]";
        if (lower.rfind("http://", 0) == 0 || lower.rfind("https://", 0) == 0) {
            std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> url(curl_url(), curl_url_cleanup);
            if (!url || curl_url_set(url.get(), CURLUPART_URL, text.c_str(), 0) != CURLUE_OK) return "[REDACTED URL]";
            for (const auto part : {CURLUPART_USER, CURLUPART_PASSWORD, CURLUPART_QUERY}) {
                char* raw = nullptr;
                if (curl_url_get(url.get(), part, &raw, 0) == CURLUE_OK) {
                    const bool present = raw && *raw;
                    curl_free(raw);
                    if (present) return "[REDACTED URL]";
                }
            }
        }
        return text;
    }
    return value;
}

static std::string display(const Json::Value& value)
{
    if (value.isNull()) return "--";
    if (value.isArray()) {
        if (value.empty()) return "[]";
        // For arrays, return type info instead of trying to serialize
        return "[" + std::to_string(value.size()) + " items]";
    }
    if (value.isBool()) return value.asBool() ? "true" : "false";
    if (value.isInt()) return std::to_string(value.asInt());
    if (value.isUInt()) return std::to_string(value.asUInt());
    if (value.isInt64()) return std::to_string(value.asInt64());
    if (value.isUInt64()) return std::to_string(value.asUInt64());
    if (value.isDouble()) return std::to_string(value.asDouble());
    if (value.isString()) return value.asString();
    
    // For objects, only serialize if small enough to avoid crash
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, value);
}

std::string textOutput(const Json::Value& snapshot)
{
    std::ostringstream output;
    for (const auto& section : snapshot["sections"]) {
        std::string sectionId = section["id"].asString();
        
        // Use DumpsysFormatter for app-related sections for proper table output
        if (sectionId == "runtime" && (section["data"].isMember("loaded_apps") || section["data"].isMember("apps"))) {
            output << DumpsysFormatter::formatRuntimeManager(section["data"]);
            continue;
        } else if (sectionId == "package" && section["data"].isMember("packages")) {
            output << DumpsysFormatter::formatPackageManager(section["data"]);
            continue;
        } else if (sectionId == "window" && section["data"].isMember("clients")) {
            output << DumpsysFormatter::formatWindowManager(section["data"]);
            continue;
        } else if (sectionId == "resource" && section["data"].isMember("stats")) {
            output << DumpsysFormatter::formatResourceMonitor(section["data"]);
            continue;
        } else if (sectionId == "platform") {
            output << DumpsysFormatter::formatPlatform(section);
            continue;
        } else if (sectionId == "input") {
            output << DumpsysFormatter::formatInputHandler(section);
            continue;
        }
        
        // Default formatting for other sections
        output << section["title"].asString() << ":\n"
               << "    Status: " << section["status"].asString() << '\n';
        for (const auto& name : section["data"].getMemberNames()) {
            const auto& data = section["data"][name];
            if (name == "plugins" && data.isArray()) {
                  std::size_t callsignWidth = 38, stateWidth = 18, modeWidth = 20;
                  for (const auto& plugin : data) {
                      callsignWidth = std::max(callsignWidth, display(plugin["callsign"]).size() + 2);
                      stateWidth = std::max(stateWidth, display(plugin["state"]).size() + 2);
                      modeWidth = std::max(modeWidth, display(plugin["mode"]).size() + 2);
                  }
                  output << "    " << std::left << std::setw(static_cast<int>(callsignWidth)) << "callsign" << std::setw(static_cast<int>(stateWidth)) << "state"
                      << std::setw(static_cast<int>(modeWidth)) << "mode" << "hosts\n";
                for (const auto& plugin : data) {
                      output << "    " << std::setw(static_cast<int>(callsignWidth)) << display(plugin["callsign"])
                          << std::setw(static_cast<int>(stateWidth)) << display(plugin["state"])
                          << std::setw(static_cast<int>(modeWidth)) << display(plugin["mode"])
                           << display(plugin["hosts"]) << '\n';
                }
            } else if (data.isArray()) {
                // Handle all arrays - check if array contains objects
                bool has_objects = false;
                std::vector<std::string> columns;
                for (const auto& row : data) {
                    if (row.isObject()) {
                        has_objects = true;
                        for (const auto& column : row.getMemberNames()) {
                            if (std::find(columns.begin(), columns.end(), column) == columns.end()) columns.push_back(column);
                        }
                    }
                }
                
                if (has_objects && !columns.empty()) {
                    // Array of objects - display as table
                    output << "    " << display(name) << ":\n";
                    std::vector<std::size_t> widths;
                    for (const auto& column : columns) {
                        auto width = display(column).size();
                        for (const auto& row : data) if (row.isObject()) width = std::max(width, display(row[column]).size());
                        widths.push_back(width);
                    }
                    for (std::size_t index = 0; index < columns.size(); ++index)
                        output << "    " << std::left << std::setw(static_cast<int>(widths[index])) << display(columns[index]) << " |";
                    output << '\n';
                    for (const auto& row : data) {
                        if (!row.isObject()) { output << "    " << display(row) << '\n'; continue; }
                        for (std::size_t index = 0; index < columns.size(); ++index)
                            output << "    " << std::left << std::setw(static_cast<int>(widths[index])) << display(row[columns[index]]) << " |";
                        output << '\n';
                    }
                } else if (data.size() > 0) {
                    // Array of primitives or empty - display as list
                    output << "    " << display(name) << ":\n";
                    for (const auto& item : data) {
                        output << "        - " << display(item) << '\n';
                    }
                } else {
                    // Empty array
                    output << "    " << display(name) << ": []\n";
                }
            } else output << "    " << display(name) << ": " << display(data) << '\n';
        }
        for (const auto& name : section["coverage"].getMemberNames()) {
            const auto& field = section["coverage"][name];
            if (field["status"] != "available")
                output << "    " << display(name) << ": -- (" << display(field["reason"]) << ")\n";
        }
        for (const auto& error : section["errors"]) output << "    Error: " << display(error) << '\n';
        output << '\n';
    }
    return output.str();
}
}