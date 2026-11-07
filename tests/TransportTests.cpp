#include "entos/RpcClient.h"
#include <arpa/inet.h>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace {
class HttpFixture {
public:
    explicit HttpFixture(std::string body, int code = 200, bool stall = false)
    {
        listener_ = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (listener_ < 0 || bind(listener_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) || listen(listener_, 1))
            throw std::runtime_error("Cannot start HTTP fixture");
        socklen_t length = sizeof(address);
        getsockname(listener_, reinterpret_cast<sockaddr*>(&address), &length);
        url = "http://127.0.0.1:" + std::to_string(ntohs(address.sin_port));
        worker_ = std::thread([this, body, code, stall] {
            pollfd waiting{listener_, POLLIN, 0};
            if (poll(&waiting, 1, 5000) <= 0) return;
            const int client = accept(listener_, nullptr, nullptr);
            if (client < 0) return;
            char buffer[4096];
            pollfd readable{client, POLLIN, 0};
            if (poll(&readable, 1, 5000) > 0) {
                const auto count = recv(client, buffer, sizeof(buffer), 0);
                if (count > 0) request.assign(buffer, static_cast<std::size_t>(count));
                if (stall) {
                    timeval timeout{2, 0};
                    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                    recv(client, buffer, sizeof(buffer), 0);
                    close(client);
                    return;
                }
                const auto response = "HTTP/1.1 " + std::to_string(code) + " Response\r\nContent-Length: "
                    + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
                std::size_t sent = 0;
                while (sent < response.size()) {
                    const auto countSent = send(client, response.data() + sent, response.size() - sent, 0);
                    if (countSent <= 0) break;
                    sent += static_cast<std::size_t>(countSent);
                }
            }
            close(client);
        });
    }
    ~HttpFixture() { finish(); close(listener_); }
    void finish() { if (worker_.joinable()) worker_.join(); }
    std::string url;
    std::string request;
private:
    int listener_ = -1;
    std::thread worker_;
};
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}
int main()
{
    try {
        HttpFixture fixture(R"({"jsonrpc":"2.0","id":1,"result":[]})");
        entos::RpcClient rpc(entos::curlTransport(entos::validateEndpoint(fixture.url), "Bearer fixture-value", 100));
        require(rpc.call("Controller.1.status", Json::Value(Json::objectValue), 2000).data.isArray(), "real HTTP RPC response");
        fixture.finish();
        require(fixture.request.find("Authorization: Bearer fixture-value\r\n") != std::string::npos, "one authorization header");
        HttpFixture oversized(std::string(128, 'x'));
        auto transport = entos::curlTransport(entos::validateEndpoint(oversized.url), "", 100, 32);
        require(transport("{}", 2000).failure == "error", "response size limit");
        HttpFixture denied("sensitive-error", 401);
        entos::RpcClient deniedRpc(entos::curlTransport(entos::validateEndpoint(denied.url), "", 100));
        const auto reply = deniedRpc.call("status", {}, 2000);
        require(reply.status == "permission-denied" && reply.error.find("sensitive") == std::string::npos, "HTTP authentication redaction");
        HttpFixture slow("", 200, true);
        auto bounded = entos::curlTransport(entos::validateEndpoint(slow.url), "", 20);
        require(bounded("{}", 50).failure == "timeout", "real HTTP deadline");
        HttpFixture invalidTls("not-tls");
        auto tlsUrl = invalidTls.url;
        tlsUrl.replace(0, 4, "https");
        auto secure = entos::curlTransport(entos::validateEndpoint(tlsUrl), "", 100);
        require(secure("{}", 1000).failure == "unavailable", "TLS failure does not accept plaintext");
        std::cout << "HTTP transport checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}