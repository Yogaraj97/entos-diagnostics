#include "entos/RpcClient.h"
#include <curl/curl.h>
#include <algorithm>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace entos {
namespace {
struct CurlRuntime {
    CurlRuntime() { if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) throw std::runtime_error("HTTP initialization failed"); }
    ~CurlRuntime() { curl_global_cleanup(); }
};
void initialize()
{
    static CurlRuntime runtime;
    (void)runtime;
}
struct Body {
    std::string text;
    std::size_t limit;
};
std::size_t receive(char* data, std::size_t size, std::size_t count, void* opaque)
{
    auto& body = *static_cast<Body*>(opaque);
    if (size && count > body.limit / size) return 0;
    const auto length = size * count;
    if (length > body.limit - body.text.size()) return 0;
    try { body.text.append(data, length); } catch (...) { return 0; }
    return length;
}
}

Endpoint validateEndpoint(const std::string& text)
{
    initialize();
    std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> parsed(curl_url(), curl_url_cleanup);
    if (!parsed || curl_url_set(parsed.get(), CURLUPART_URL, text.c_str(), 0) != CURLUE_OK)
        throw std::runtime_error("Invalid Thunder URL");
    const auto part = [&](CURLUPart name) {
        char* raw = nullptr;
        if (curl_url_get(parsed.get(), name, &raw, 0) != CURLUE_OK) return std::string();
        std::unique_ptr<char, decltype(&curl_free)> storage(raw, curl_free);
        return std::string(raw);
    };
    const auto scheme = part(CURLUPART_SCHEME);
    if ((scheme != "http" && scheme != "https") || !part(CURLUPART_USER).empty()
            || !part(CURLUPART_PASSWORD).empty() || !part(CURLUPART_QUERY).empty()
            || !part(CURLUPART_FRAGMENT).empty())
        throw std::runtime_error("Thunder URL must be HTTP(S) without credentials, query, or fragment");
    const auto host = part(CURLUPART_HOST);
    auto url = part(CURLUPART_URL);
    while (!url.empty() && url.back() == '/') url.pop_back();
    if (url.size() < 8 || url.substr(url.size() - 8) != "/jsonrpc") url += "/jsonrpc";
    return {url, host == "127.0.0.1" || host == "localhost" || host == "[::1]" || host == "::1"};
}

void validateToken(const std::string& token)
{
    if (token.size() > 4096 || std::any_of(token.begin(), token.end(), [](unsigned char value) {
        return value < 32 || value == 127;
    })) throw std::runtime_error("Invalid authentication header");
}

std::string readToken(const std::string& path)
{
    const int descriptor = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (descriptor < 0) throw std::runtime_error("Cannot open protected token file");
    const auto closeDescriptor = [](int* value) { close(*value); delete value; };
    std::unique_ptr<int, decltype(closeDescriptor)> guard(new int(descriptor), closeDescriptor);
    struct stat metadata{};
    if (fstat(descriptor, &metadata) || !S_ISREG(metadata.st_mode) || metadata.st_uid != geteuid()
            || (metadata.st_mode & 077) != 0 || metadata.st_size > 4096)
        throw std::runtime_error("Token file must be owner-only, regular, and owned by the current user");
    std::string token;
    char buffer[4096];
    while (true) {
        const auto length = read(descriptor, buffer, sizeof(buffer));
        if (length < 0) throw std::runtime_error("Cannot read token file");
        if (length == 0) break;
        token.append(buffer, static_cast<std::size_t>(length));
        if (token.size() > 4096) throw std::runtime_error("Token file exceeds size limit");
    }
    if (!token.empty() && token.back() == '\n') token.pop_back();
    validateToken(token);
    return token;
}

Transport curlTransport(const Endpoint& endpoint, const std::string& token, long connectMs,
                        std::size_t maxBytes)
{
    initialize();
    validateToken(token);
    return [endpoint, token, connectMs, maxBytes](const std::string& request, long timeoutMs) {
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(), curl_easy_cleanup);
        if (!handle) return HttpReply{0, "", "error"};
        curl_slist* rawHeaders = curl_slist_append(nullptr, "Content-Type: application/json");
        if (!rawHeaders) return HttpReply{0, "", "error"};
        if (!token.empty()) {
            auto* appended = curl_slist_append(rawHeaders, ("Authorization: " + token).c_str());
            if (!appended) { curl_slist_free_all(rawHeaders); return HttpReply{0, "", "error"}; }
            rawHeaders = appended;
        }
        std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers(rawHeaders, curl_slist_free_all);
        Body body{"", maxBytes};
        curl_easy_setopt(handle.get(), CURLOPT_URL, endpoint.url.c_str());
        curl_easy_setopt(handle.get(), CURLOPT_HTTPHEADER, headers.get());
        curl_easy_setopt(handle.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(handle.get(), CURLOPT_POSTFIELDS, request.data());
        curl_easy_setopt(handle.get(), CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(request.size()));
        curl_easy_setopt(handle.get(), CURLOPT_CONNECTTIMEOUT_MS, std::min(connectMs, timeoutMs));
        curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT_MS, timeoutMs);
        curl_easy_setopt(handle.get(), CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, receive);
        curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &body);
        const auto code = curl_easy_perform(handle.get());
        if (code != CURLE_OK)
            return HttpReply{0, "", code == CURLE_OPERATION_TIMEDOUT ? "timeout" : code == CURLE_WRITE_ERROR ? "error" : "unavailable"};
        long httpCode = 0;
        curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &httpCode);
        return HttpReply{httpCode, std::move(body.text), ""};
    };
}
}