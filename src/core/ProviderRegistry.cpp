#include "entos/ProviderRegistry.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <set>
#include <stdexcept>

namespace entos {
const std::vector<SectionContract>& sectionContracts()
{
    static const std::vector<SectionContract> contracts = {
        {"config", "CONFIG", "ASAppManager", {"files", "effectiveConfig"}},
        {"dial", "DIAL", "DIAL/XCast", {"state", "enabled", "deviceDescription", "wakeup"}},
        {"iui", "DYNAMIC IUI MANAGER", "ASAppManager", {"enabled", "selectorRFC"}},
        {"forwarded-launch", "FORWARDED LAUNCH MANAGER", "ASAppManager", {"enabled", "pendingLaunch"}},
        {"input", "INPUT HANDLER", "RDKWindowManager", {"devices", "activeKeymap"}},
        {"package", "PACKAGE MANAGER", "AppPackageManager", {"packages", "installer", "cache", "mounts"}},
        {"pathway", "PATHWAY MANAGER", "ASAppManager", {"enabled", "lastFetch", "installations"}},
        {"platform", "PLATFORM", "Platform", {"kernel", "uptime", "memory", "filesystems", "firmware"}},
        {"power", "POWER STATE MANAGER", "Platform", {"currentState", "previousState"}},
        {"removal", "REMOVE UNUSED APPS MANAGER", "ASCatalogManager", {"lastRun", "summary"}},
        {"runtime", "RUNTIME MANAGER", "AppManager/LifecycleManager/RuntimeManager", {"apps", "lifecycle", "runtimeInfo", "sessions"}},
        {"softcat", "SOFTCAT", "ASCatalogManager", {"state", "lastRefresh", "nextRefresh", "packages"}},
        {"startup", "STARTUP APP LAUNCHER", "SceneSet/ASAppManager", {"service", "pending", "recentApps"}},
        {"system-app", "SYSTEM APP MANAGER", "ASAppManager", {"services", "launchTimes"}},
        {"thunder", "THUNDER SERVICE MANAGER", "Controller", {"plugins"}},
        {"window", "WINDOW MANAGER", "RDKWindowManager/compositor", {"clients", "focus", "surfaces", "players", "vnc"}},
        {"gateway", "APP GATEWAY", "AppGateway", {"plugins", "connections", "routes"}},
        {"resource", "RESOURCE MONITOR", "ResourceMonitor", {"stats"}}
    };
    return contracts;
}

Json::Value parseJson(const std::string& text)
{
    Json::CharReaderBuilder builder;
    builder["allowComments"] = false;
    builder["rejectDupKeys"] = true;
    builder["failIfExtra"] = true;
    builder["allowTrailingCommas"] = false;
    builder["allowSingleQuotes"] = false;
    builder["stackLimit"] = 64;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value result;
    std::string errors;
    if (!reader->parse(text.data(), text.data() + text.size(), &result, &errors))
        throw std::runtime_error("Invalid JSON document");
    return result;
}

std::vector<Provider> parseProviders(const Json::Value& root)
{
    if (!root.isObject() || !root["schemaVersion"].isInt() || root["schemaVersion"].asInt() != 1
            || !root["providers"].isArray()) throw std::runtime_error("Invalid provider schema version or list");
    static const std::set<std::string> allowed = {
        "getInstalledApps", "getLoadedApps", "listPackages", "getApps", "GetStats",
        "GetMemInfo", "GetSwapUsed", "GetFlashSpace", "getMaxRunningApps",
        "getMaxHibernatedApps", "getMaxHibernatedFlashUsage", "getMaxInactiveRamUsage"
    };
    std::set<std::string> identities;
    std::vector<Provider> result;
    for (const auto& entry : root["providers"]) {
        if (!entry.isObject()) throw std::runtime_error("Provider must be an object");
        for (const auto* key : {"id", "section", "interfaceVersion", "operation", "resultType", "evidence", "verification"}) {
            if (!entry[key].isString() || entry[key].asString().empty())
                throw std::runtime_error("Missing provider contract field");
        }
        Provider provider;
        provider.id = entry["id"].asString();
        provider.section = entry["section"].asString();
        provider.version = entry["interfaceVersion"].asString();
        provider.operation = entry["operation"].asString();
        provider.resultType = entry["resultType"].asString();
        provider.evidence = entry["evidence"].asString();
        if (provider.id.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-") != std::string::npos)
            throw std::runtime_error("Invalid provider ID");
        if (entry["verification"] != "verified" && entry["verification"] != "provisional")
            throw std::runtime_error("Invalid verification status");
        if (!identities.insert(provider.id).second) throw std::runtime_error("Duplicate provider ID");
        if (!allowed.count(provider.operation) || entry["readOnly"] != Json::Value(true))
            throw std::runtime_error("Operation is not in the read-only allowlist");
        if (!std::all_of(provider.version.begin(), provider.version.end(), [](unsigned char value) {
            return value >= '0' && value <= '9';
        })) throw std::runtime_error("Invalid interface version");
        if (std::none_of(sectionContracts().begin(), sectionContracts().end(), [&](const auto& section) {
            return section.id == provider.section;
        })) throw std::runtime_error("Unknown provider section");
        if (provider.resultType != "object" && provider.resultType != "array")
            throw std::runtime_error("Unsupported response contract");
        if (!entry["params"].isObject() || !entry["enabled"].isBool()
                || !entry["callsigns"].isArray() || entry["callsigns"].empty())
            throw std::runtime_error("Invalid provider parameters or callsigns");
        provider.params = entry["params"];
        provider.enabled = entry["enabled"].asBool();
        if (provider.enabled && entry["verification"].asString() != "verified")
            throw std::runtime_error("Unverified provider cannot be enabled");
        for (const auto& callsign : entry["callsigns"]) {
            if (!callsign.isString()) throw std::runtime_error("Invalid callsign");
            const auto name = callsign.asString();
            if (name.empty() || !std::all_of(name.begin(), name.end(), [](unsigned char value) {
                        return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z')
                            || (value >= '0' && value <= '9') || value == '.' || value == '_' || value == '-';
                    })) throw std::runtime_error("Invalid callsign");
            provider.callsigns.push_back(name);
        }
        if (entry.isMember("fieldMappings")) {
            if (!entry["fieldMappings"].isObject()) throw std::runtime_error("Invalid field mappings");
            const auto contract = std::find_if(sectionContracts().begin(), sectionContracts().end(), [&](const auto& section) { return section.id == provider.section; });
            for (const auto& field : entry["fieldMappings"].getMemberNames()) {
                const auto& mapping = entry["fieldMappings"][field];
                if (std::find(contract->fields.begin(), contract->fields.end(), field) == contract->fields.end()
                        || !mapping.isObject() || !mapping["path"].isArray() || !mapping["embeddedJson"].isBool()) {
                    std::string error = "Invalid field mapping contract for provider: " + provider.id + 
                                        ", section: " + provider.section + 
                                        ", field: " + field;
                    if (!mapping.isObject())
                        error += " (mapping not object)";
                    else if (!mapping["path"].isArray())
                        error += " (path not array)";
                    else if (!mapping["embeddedJson"].isBool())
                        error += " (embeddedJson not boolean)";
                    else
                        error += " (field '" + field + "' not in section contract)";
                    throw std::runtime_error(error);
                }
                Provider::Mapping parsed;
                parsed.embeddedJson = mapping["embeddedJson"].asBool();
                for (const auto& key : mapping["path"]) {
                    if (!key.isString()) throw std::runtime_error("Mapping path must contain object keys");
                    parsed.path.push_back(key.asString());
                }
                provider.fields[field] = std::move(parsed);
            }
        }
        if (provider.enabled && provider.fields.empty()) throw std::runtime_error("Enabled providers require reviewed field mappings");
        result.push_back(std::move(provider));
    }
    return result;
}

std::vector<Provider> mergeProviderOverrides(std::vector<Provider> baseProviders, const std::string& overridePath)
{
    // Try to load override file from /opt/entos-diagnostics/providers.json.override
    std::ifstream override_file(overridePath);
    if (!override_file) {
        // Override file doesn't exist, return base providers unchanged
        return baseProviders;
    }
    
    // Read override file
    std::string override_document;
    char buffer[4096];
    while (override_file.read(buffer, sizeof(buffer)) || override_file.gcount()) {
        override_document.append(buffer, static_cast<std::size_t>(override_file.gcount()));
        if (override_document.size() > 1024 * 1024) {
            throw std::runtime_error("Provider override file exceeds size limit");
        }
    }
    
    // Parse override JSON
    const auto override_root = parseJson(override_document);
    const auto override_providers = parseProviders(override_root);
    
    // Create a map of override providers by ID for quick lookup
    std::map<std::string, Provider> override_map;
    for (const auto& provider : override_providers) {
        override_map[provider.id] = provider;
    }
    
    // Merge: for each base provider, check if there's an override
    std::vector<Provider> merged;
    for (auto provider : baseProviders) {
        const auto it = override_map.find(provider.id);
        if (it != override_map.end()) {
            // Override exists: use the override provider instead
            merged.push_back(it->second);
            override_map.erase(it);  // Mark as used
        } else {
            // No override: keep base provider
            merged.push_back(provider);
        }
    }
    
    // Add any remaining override providers that didn't exist in base
    for (const auto& [id, provider] : override_map) {
        merged.push_back(provider);
    }
    
    return merged;
}
}