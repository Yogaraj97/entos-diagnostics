#include "entos/Collector.h"
#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
struct Options {
    std::string command = "dumpsys";
    std::string format = "text";
    std::string url = "http://127.0.0.1:9998";
    std::string tokenFile;
    std::filesystem::path profile;
    std::filesystem::path procRoot = "/proc";
    std::vector<std::string> sections;
    std::vector<std::filesystem::path> configFiles;
    long connectMs = 1000;
    long requestMs = 3000;
    long totalMs = 15000;
};
long number(const std::string& text)
{
    long value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() || value < 1 || value > 600000)
        throw std::runtime_error("Timeout must be an integer from 1 to 600000 milliseconds");
    return value;
}
void usage()
{
    std::cout << "Usage: entos-diagnostics [dumpsys|plugins|list] [options]\n"
                 "  --section ID       Repeat to select dumpsys sections\n"
                 "  --format text|json Output format\n"
                 "  --url URL          Thunder base URL (or THUNDER_URL)\n"
                 "  --profile FILE     Reviewed versioned provider profile\n"
                 "  --token-file FILE  Owner-only authorization header file\n"
                 "  --connect-ms N     Connection timeout\n"
                 "  --request-ms N     Per-call timeout\n"
                 "  --timeout-ms N     Total snapshot budget\n"
                 "  --proc-root DIR    Native procfs root (fixture testing)\n"
                 "  --config-file FILE Ordered declared JSON/YAML source; repeatable\n"
                 "  --help             Show this help\n"
                 "Exit: 0 complete, 1 partial diagnostic result, 2 invalid input/configuration.\n";
}
Options parse(int count, char** arguments)
{
    Options options;
    if (const auto* environment = std::getenv("THUNDER_URL")) options.url = environment;
    options.profile = ENTOS_INSTALLED_PROFILE;
    const auto adjacent = std::filesystem::absolute(arguments[0]).parent_path() / "providers.json";
    std::error_code error;
    if (std::filesystem::is_regular_file(adjacent, error)) options.profile = adjacent;
    int index = 1;
    if (index < count && arguments[index][0] != '-') options.command = arguments[index++];
    for (; index < count; ++index) {
        const std::string option = arguments[index];
        if (index + 1 >= count) throw std::runtime_error("Option requires a value");
        const std::string value = arguments[++index];
        if (option == "--section") options.sections.push_back(value);
        else if (option == "--format") options.format = value;
        else if (option == "--url") options.url = value;
        else if (option == "--profile") options.profile = value;
        else if (option == "--token-file") options.tokenFile = value;
        else if (option == "--proc-root") options.procRoot = value;
        else if (option == "--config-file") options.configFiles.emplace_back(value);
        else if (option == "--connect-ms") options.connectMs = number(value);
        else if (option == "--request-ms") options.requestMs = number(value);
        else if (option == "--timeout-ms") options.totalMs = number(value);
        else throw std::runtime_error("Unknown option; run --help");
    }
    if (options.command != "dumpsys" && options.command != "plugins" && options.command != "list")
        throw std::runtime_error("Unknown command; use dumpsys, plugins, or list");
    if (options.format != "text" && options.format != "json") throw std::runtime_error("Format must be text or json");
    if (options.command != "dumpsys" && !options.sections.empty()) throw std::runtime_error("--section is only valid with dumpsys");
    for (const auto& name : options.sections) {
        if (std::none_of(entos::sectionContracts().begin(), entos::sectionContracts().end(), [&](const auto& contract) { return contract.id == name; }))
            throw std::runtime_error("Unknown section; run list for valid section names");
    }
    return options;
}
}

int main(int count, char** arguments)
{
    try {
        for (int index = 1; index < count; ++index) {
            if (std::string(arguments[index]) == "--help") { usage(); return 0; }
        }
        const auto options = parse(count, arguments);
        if (options.command == "list") {
            Json::Value manifest;
            manifest["schemaVersion"] = 1;
            manifest["sections"] = Json::Value(Json::arrayValue);
            for (const auto& contract : entos::sectionContracts()) {
                Json::Value section;
                section["id"] = contract.id;
                section["title"] = contract.title;
                section["owner"] = contract.owner;
                section["fields"] = Json::Value(Json::arrayValue);
                for (const auto& field : contract.fields) section["fields"].append(field);
                manifest["sections"].append(section);
                if (options.format == "text") std::cout << contract.id << "  " << contract.title << '\n';
            }
            if (options.format == "json") std::cout << manifest.toStyledString();
            return 0;
        }
        const auto endpoint = entos::validateEndpoint(options.url);
        std::ifstream input(options.profile);
        if (!input) throw std::runtime_error("Cannot read provider profile; specify --profile");
        std::string document;
        char buffer[4096];
        while (input.read(buffer, sizeof(buffer)) || input.gcount()) {
            document.append(buffer, static_cast<std::size_t>(input.gcount()));
            if (document.size() > 1024 * 1024) throw std::runtime_error("Provider profile exceeds size limit");
        }
        auto providers = entos::parseProviders(entos::parseJson(document));
        
        // Merge provider overrides from /opt/entos-diagnostics/providers.json.override
        providers = entos::mergeProviderOverrides(providers, "/opt/entos-diagnostics/providers.json.override");
        
        std::string token;
        if (!options.tokenFile.empty()) token = entos::readToken(options.tokenFile);
        else if (const auto* environment = std::getenv("TOKEN")) token = environment;
        entos::validateToken(token);
        entos::RpcClient rpc(entos::curlTransport(endpoint, token, options.connectMs));
        entos::CollectionContext context{endpoint, rpc, entos::Clock::now() + std::chrono::milliseconds(options.totalMs)};
        context.requestMs = options.requestMs;
        context.procRoot = options.procRoot;
        context.providers = providers;
        context.configFiles = {"/etc/sky/aisettings.json", "/etc/sky/aisettings.overrides/platform.json", "/etc/sky/aisettings.overrides/vendor-gpu.json"};
        if (!options.configFiles.empty()) context.configFiles = options.configFiles;
        const auto selected = options.command == "plugins" ? std::vector<std::string>{"thunder"} : options.sections;
        auto snapshot = entos::CollectionEngine().collect(context, selected);
        std::vector<std::string> secrets{token};
        if (token.rfind("Bearer ", 0) == 0) secrets.push_back(token.substr(7));
        snapshot = entos::sanitize(snapshot, secrets);
        if (options.format == "json") std::cout << snapshot.toStyledString();
        else std::cout << entos::textOutput(snapshot);
        const auto& sections = snapshot["sections"];
        return std::all_of(sections.begin(), sections.end(), [](const auto& section) { return section["status"] == "ok"; }) ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 2;
    }
}