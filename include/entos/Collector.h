#pragma once
#include "entos/RpcClient.h"
#include "entos/Snapshot.h"
#include "entos/ProcessSnapshot.h"
#include <filesystem>
#include <map>

namespace entos {
struct CollectionContext {
    Endpoint endpoint;
    RpcClient& rpc;
    Clock::time_point deadline;
    long requestMs = 3000;
    std::size_t limit = 64;
    std::filesystem::path procRoot = "/proc";
    std::filesystem::path sysRoot = "/sys";
    std::vector<std::filesystem::path> configFiles;
    std::vector<Provider> providers;
    std::vector<Process> processes;
    bool processVisibility = false;
    long remainingMs() const;
};
using Collector = std::function<void(Json::Value&, CollectionContext&)>;
class CollectionEngine {
public:
    CollectionEngine();
    void registerCollector(const std::string& section, Collector collector);
    Json::Value collect(CollectionContext& context, const std::vector<std::string>& sections) const;
private:
    std::map<std::string, Collector> collectors_;
};
void collectPlugins(Json::Value& section, CollectionContext& context);
void collectPlatform(Json::Value& section, CollectionContext& context);
void collectConfiguration(Json::Value& section, CollectionContext& context);
void collectInput(Json::Value& section, CollectionContext& context);
void collectApplication(Json::Value& section, CollectionContext& context);
}