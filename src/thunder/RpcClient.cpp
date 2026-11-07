#include "entos/RpcClient.h"
#include "entos/ProviderRegistry.h"

namespace entos {
RpcClient::RpcClient(Transport transport) : transport_(std::move(transport)) {}

RpcReply RpcClient::call(const std::string& method, const Json::Value& params, long timeoutMs)
{
    if (timeoutMs <= 0) return {"timeout", {}, "Snapshot deadline exhausted", 0};
    Json::Value request;
    const auto identity = ++nextId_;
    request["jsonrpc"] = "2.0";
    request["id"] = identity;
    request["method"] = method;
    request["params"] = params;
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    const auto response = transport_(Json::writeString(writer, request), timeoutMs);
    if (!response.failure.empty()) return {response.failure, {}, "HTTP transport failed", 0};
    if (response.code == 401 || response.code == 403) return {"permission-denied", {}, "Authentication denied", 0};
    if (response.code < 200 || response.code >= 300) return {"unavailable", {}, "HTTP request failed", 0};
    try {
        const auto envelope = parseJson(response.body);
        if (!envelope.isObject() || envelope["jsonrpc"] != "2.0" || !envelope["id"].isUInt64()
                || envelope["id"].asUInt64() != identity
                || envelope.isMember("result") == envelope.isMember("error"))
            return {"error", {}, "Invalid JSON-RPC envelope", 0};
        if (envelope.isMember("error")) {
                if (!envelope["error"].isObject() || !envelope["error"]["code"].isInt()
                    || !envelope["error"]["message"].isString())
                return {"error", {}, "Invalid JSON-RPC error", 0};
            const auto code = envelope["error"]["code"].asInt();
            return {code == -32601 ? "unsupported" : "error", {}, "JSON-RPC call failed", code};
        }
        return {"ok", envelope["result"], "", 0};
    } catch (const std::exception&) {
        return {"error", {}, "Malformed JSON-RPC response", 0};
    }
}
}