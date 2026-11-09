#include "entos/Collector.h"
#include <algorithm>

namespace entos {
void collectApplication(Json::Value& section, CollectionContext& context)
{
    for (const auto& provider : context.providers) {
        if (provider.section != section["id"].asString()) continue;
        if (!provider.enabled) {
            section["errors"].append("Provider " + provider.id + " disabled: target contract verification pending");
            continue;
        }
        RpcReply reply;
        for (const auto& callsign : provider.callsigns) {
            reply = context.rpc.call(callsign + "." + provider.version + "." + provider.operation,
                                     provider.params, std::min(context.requestMs, context.remainingMs()));
            if (reply.code != -32601) break;
        }
        if (reply.status != "ok") {
            section["status"] = reply.status;
            section["errors"].append(provider.id + ": " + reply.error);
            continue;
        }
        if ((provider.resultType == "object" && !reply.data.isObject())
                || (provider.resultType == "array" && !reply.data.isArray())) {
            section["errors"].append(provider.id + ": result violates configured response contract");
            section["status"] = "error";
            continue;
        }
        for (const auto& field : provider.fields) {
            Json::Value value = reply.data;
            bool present = true;
            
            // Navigate through the path (if any)
            for (const auto& key : field.second.path) {
                if (!value.isObject()) { present = false; break; }
                if (!value.isMember(key)) { present = false; break; }
                value = value[key];
            }
            
            // If path navigation failed or value is null, skip this field
            if (!present || value.isNull()) {
                section["errors"].append(provider.id + ": required mapped response field missing");
                continue;
            }
            
            // Handle embedded JSON (convert JSON to string representation)
            if (field.second.embeddedJson) {
                if (!value.isString()) {
                    // Convert non-string values to JSON string
                    try { value = Json::Value(Json::writeString(Json::StreamWriterBuilder(), value)); }
                    catch (...) { section["errors"].append("Failed to serialize JSON field"); continue; }
                } else {
                    // Verify it's valid JSON string
                    try { parseJson(value.asString()); }
                    catch (...) { section["errors"].append("Invalid declared embedded JSON field"); continue; }
                }
            }
            
            recordField(section, field.first, value, provider.evidence);
        }
    }
}
}