#pragma once
#include <json/json.h>
#include <functional>

namespace entos {
struct HttpReply {
    long code = 0;
    std::string body;
    std::string failure;
};
struct RpcReply {
    std::string status = "ok";
    Json::Value data;
    std::string error;
    int code = 0;
};
using Transport = std::function<HttpReply(const std::string&, long)>;
class RpcClient {
public:
    explicit RpcClient(Transport transport);
    RpcReply call(const std::string& method, const Json::Value& params, long timeoutMs);
private:
    Transport transport_;
    Json::UInt64 nextId_ = 0;
};
struct Endpoint {
    std::string url;
    bool local = false;
};
Endpoint validateEndpoint(const std::string& url);
std::string readToken(const std::string& path);
void validateToken(const std::string& token);
Transport curlTransport(const Endpoint& endpoint, const std::string& token, long connectMs,
                        std::size_t maxBytes = 2 * 1024 * 1024);
}