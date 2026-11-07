#include "entos/Collector.h"
#include <algorithm>
#include <map>

namespace entos {
void collectPlugins(Json::Value& section, CollectionContext& context)
{
    const auto reply = context.rpc.call("Controller.1.status", Json::Value(Json::objectValue),
                                        std::min(context.requestMs, context.remainingMs()));
    if (reply.status != "ok") {
        section["status"] = reply.status;
        section["errors"].append(reply.error);
        section["coverage"]["plugins"]["status"] = reply.status;
        section["coverage"]["plugins"]["reason"] = reply.error;
        return;
    }
    const auto inventory = reply.data.isArray() ? reply.data
        : reply.data.isObject() && reply.data["plugins"].isArray() ? reply.data["plugins"] : Json::Value();
    if (!inventory.isArray()) {
        section["status"] = "error";
        section["errors"].append("Unsupported Controller status result shape");
        return;
    }
    if (context.endpoint.local) {
        context.processes = readProcesses(context.procRoot, &context.processVisibility);
    }
    std::map<std::string, Json::Value> plugins;
    for (const auto& entry : inventory) {
        if (!entry.isObject() || !entry["callsign"].isString() || entry["callsign"].asString().empty()) {
            section["errors"].append("Malformed plugin inventory entry omitted");
            continue;
        }
        const auto name = entry["callsign"].asString();
        if (plugins.count(name)) {
            if (plugins[name]["state"] != entry["state"]) {
                plugins[name]["state"] = Json::Value();
                plugins[name]["conflictingEvidence"] = true;
                section["errors"].append("Conflicting duplicate plugin state");
            }
            continue;
        }
        Json::Value plugin;
        plugin["callsign"] = name;
        for (const auto* key : {"state", "lifecycle", "status", "phase"}) {
            if (entry[key].isString()) plugin[key] = entry[key];
        }
        plugin["mapped"] = std::any_of(context.providers.begin(), context.providers.end(), [&](const auto& provider) {
            return std::find(provider.callsigns.begin(), provider.callsigns.end(), name) != provider.callsigns.end();
        });
        plugins[name] = std::move(plugin);
    }
    Json::Value output(Json::arrayValue);
    for (auto& entry : plugins) {
        auto& plugin = entry.second;
        plugin["hosts"] = Json::Value(Json::arrayValue);
        plugin["mode"] = "UNKNOWN";
        plugin["modeReason"] = context.endpoint.local ? "Process visibility incomplete or unavailable" : "Remote target has no target-side process provider";
        if (context.endpoint.local) {
            const auto mode = executionMode(entry.first, plugin["state"].isString() ? plugin["state"].asString() : "", context.processes);
            if (mode.mode == "OOP" || context.processVisibility) {
                plugin["mode"] = mode.mode;
                plugin["modeReason"] = mode.mode == "INPROC_INFERRED" ? "Active plugin and visible framework; shared hosts remain possible" : "Target-local process/state evidence";
            }
            for (const auto pid : mode.hosts) {
                const auto process = std::find_if(context.processes.begin(), context.processes.end(), [&](const auto& candidate) { return candidate.pid == pid; });
                Json::Value host;
                host["pid"] = pid;
                host["ppid"] = process == context.processes.end() ? 0 : process->parent;
                plugin["hosts"].append(host);
            }
        }
        if (plugin.get("conflictingEvidence", false).asBool()) plugin["mode"] = "UNKNOWN";
        output.append(plugin);
    }
    recordField(section, "plugins", output, "Controller.1.status; target-local procfs where available");
}
}