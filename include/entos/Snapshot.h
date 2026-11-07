#pragma once
#include "entos/ProviderRegistry.h"
#include <chrono>

namespace entos {
using Clock = std::chrono::steady_clock;
std::string timestamp();
Json::Value newSection(const SectionContract& contract);
void recordField(Json::Value& section, const std::string& name, const Json::Value& value,
                 const std::string& source);
void completeSection(Json::Value& section);
Json::Value sanitize(const Json::Value& value, const std::vector<std::string>& secrets = {});
std::string textOutput(const Json::Value& snapshot);
}