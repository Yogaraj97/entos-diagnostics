#include "entos/Collector.h"
#include <algorithm>
#include <stdexcept>

namespace entos {
long CollectionContext::remainingMs() const
{
    return std::max(0L, static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count()));
}
CollectionEngine::CollectionEngine()
{
    registerCollector("thunder", collectPlugins);
    registerCollector("platform", collectPlatform);
    registerCollector("config", collectConfiguration);
    registerCollector("input", collectInput);
    for (const auto& contract : sectionContracts()) {
        if (!collectors_.count(contract.id)) registerCollector(contract.id, collectApplication);
    }
}
void CollectionEngine::registerCollector(const std::string& section, Collector collector)
{
    collectors_[section] = std::move(collector);
}
Json::Value CollectionEngine::collect(CollectionContext& context, const std::vector<std::string>& selected) const
{
    for (const auto& name : selected) {
        if (!collectors_.count(name)) throw std::runtime_error("Unknown section");
    }
    Json::Value snapshot;
    snapshot["schemaVersion"] = 1;
    snapshot["target"] = context.endpoint.url;
    snapshot["localCollectorsEnabled"] = context.endpoint.local;
    snapshot["startedAt"] = timestamp();
    snapshot["sections"] = Json::Value(Json::arrayValue);
    const auto started = Clock::now();
    for (const auto& contract : sectionContracts()) {
        if (!selected.empty() && std::find(selected.begin(), selected.end(), contract.id) == selected.end()) continue;
        auto section = newSection(contract);
        if (context.remainingMs() == 0) {
            section["status"] = "timeout";
            section["errors"].append("Snapshot deadline exhausted before collection");
        } else {
            try { collectors_.at(contract.id)(section, context); }
            catch (const std::exception&) {
                section["status"] = "error";
                section["errors"].append("Collector failed; unrelated sections preserved");
            }
        }
        completeSection(section);
        snapshot["sections"].append(section);
    }
    snapshot["elapsedMs"] = static_cast<Json::Int64>(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - started).count());
    return snapshot;
}
}