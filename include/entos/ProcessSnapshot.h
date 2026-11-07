#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace entos {
struct Process {
    int pid = 0;
    int parent = 0;
    std::vector<std::string> arguments;
};
struct ExecutionMode {
    std::string mode = "UNKNOWN";
    std::vector<int> hosts;
};
std::vector<Process> readProcesses(const std::filesystem::path& root, bool* complete = nullptr);
ExecutionMode executionMode(const std::string& callsign, const std::string& state,
                            const std::vector<Process>& processes);
}