#pragma once
#include <json/json.h>
#include <map>
#include <string>
#include <vector>

namespace entos {
struct SectionContract {
    std::string id;
    std::string title;
    std::string owner;
    std::vector<std::string> fields;
};
struct Provider {
    struct Mapping {
        std::vector<std::string> path;
        bool embeddedJson = false;
    };
    std::string id;
    std::string section;
    std::vector<std::string> callsigns;
    std::string version;
    std::string operation;
    Json::Value params;
    std::string resultType;
    std::string evidence;
    bool enabled = false;
    std::map<std::string, Mapping> fields;
};
const std::vector<SectionContract>& sectionContracts();
Json::Value parseJson(const std::string& text);
std::vector<Provider> parseProviders(const Json::Value& root);
std::vector<Provider> mergeProviderOverrides(std::vector<Provider> baseProviders, const std::string& overridePath);
}