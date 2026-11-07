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
            for (const auto& key : field.second.path) {
                if (!value.isObject() || !value.isMember(key)) { present = false; break; }
                const auto next = value[key];
                value = next;
            }
            if (!present || value.isNull()) {
                section["errors"].append(provider.id + ": required mapped response field missing");
                continue;
            }
            if (field.second.embeddedJson) {
                if (!value.isString()) { section["errors"].append("Embedded JSON contract requires a string"); continue; }
                try { value = parseJson(value.asString()); }
                catch (...) { section["errors"].append("Invalid declared embedded JSON field"); continue; }
            }
            recordField(section, field.first, value, provider.evidence);
        }
    }
}
}