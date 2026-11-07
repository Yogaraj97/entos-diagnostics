#include "entos/ProcessSnapshot.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace entos {
std::vector<Process> readProcesses(const std::filesystem::path& root, bool* complete)
{
    std::vector<Process> result;
    std::error_code error;
    std::filesystem::directory_iterator iterator(root, error);
    bool visible = !error;
    for (; !error && iterator != std::filesystem::directory_iterator(); iterator.increment(error)) {
        const auto& entry = *iterator;
        const auto name = entry.path().filename().string();
        if (name.empty() || !std::all_of(name.begin(), name.end(), [](unsigned char value) {
                return value >= '0' && value <= '9';
            })) continue;
        Process process;
        try { process.pid = std::stoi(name); } catch (...) { continue; }
        std::ifstream command(entry.path() / "cmdline", std::ios::binary);
        if (!command) { visible = false; continue; }
        std::string argument;
        std::size_t bytes = 0;
        char character;
        while (bytes++ < 65536 && command.get(character)) {
            if (character == '\0') { process.arguments.push_back(argument); argument.clear(); }
            else argument += character;
        }
        if (bytes > 65536) visible = false;
        if (process.arguments.empty()) continue;
        std::ifstream status(entry.path() / "status");
        if (!status) visible = false;
        std::string line;
        while (std::getline(status, line)) {
            if (line.rfind("PPid:", 0) == 0) {
                std::istringstream(line.substr(5)) >> process.parent;
            }
        }
        result.push_back(std::move(process));
    }
    if (complete) *complete = visible && !error;
    return result;
}

ExecutionMode executionMode(const std::string& callsign, const std::string& state,
                            const std::vector<Process>& processes)
{
    ExecutionMode result;
    auto normalized = state;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    bool framework = false;
    for (const auto& process : processes) {
        if (process.arguments.empty()) continue;
        const auto executable = std::filesystem::path(process.arguments.front()).filename().string();
        if (executable == "WPEFramework" || executable == "Thunder") framework = true;
        if (executable != "WPEProcess" && executable != "ThunderPlugin") continue;
        for (std::size_t index = 1; index < process.arguments.size(); ++index) {
            const auto& argument = process.arguments[index];
            if ((argument == "-C" && index + 1 < process.arguments.size()
                    && process.arguments[index + 1] == callsign)
                || argument == "-C" + callsign) {
                result.hosts.push_back(process.pid);
                break;
            }
        }
    }
    if (!result.hosts.empty()) result.mode = "OOP";
    else if (normalized == "deactivated" || normalized == "unavailable" || normalized == "destroyed")
        result.mode = "NOT_RUNNING";
    else if (framework && (normalized == "activated" || normalized == "resumed"))
        result.mode = "INPROC_INFERRED";
    return result;
}
}