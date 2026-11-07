#include "entos/Collector.h"
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>
#ifdef ENTOS_ENABLE_YAML
#include <yaml-cpp/yaml.h>
#endif

namespace entos {
#ifdef ENTOS_ENABLE_YAML
static Json::Value yamlValue(const YAML::Node& node, unsigned depth, std::size_t& budget)
{
    if (depth > 32 || node.size() > 4096 || budget < 64) throw std::runtime_error("error");
    budget -= 64;
    if (node.IsNull()) return Json::Value();
    if (node.IsScalar()) {
        const auto value = node.as<std::string>();
        if (value.size() > budget) throw std::runtime_error("error");
        budget -= value.size();
        return value;
    }
    if (node.IsSequence()) {
        Json::Value result(Json::arrayValue);
        for (const auto& child : node) result.append(yamlValue(child, depth + 1, budget));
        return result;
    }
    Json::Value result(Json::objectValue);
    for (const auto& child : node) {
        const auto key = child.first.as<std::string>();
        if (key.size() > budget) throw std::runtime_error("error");
        budget -= key.size();
        result[key] = yamlValue(child.second, depth + 1, budget);
    }
    return result;
}
#endif
static std::string readBounded(const std::filesystem::path& path)
{
    const int descriptor = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    if (descriptor < 0) throw std::runtime_error(errno == EACCES || errno == EPERM ? "permission-denied" : "unsupported");
    const auto closeDescriptor = [](int* value) { close(*value); delete value; };
    std::unique_ptr<int, decltype(closeDescriptor)> guard(new int(descriptor), closeDescriptor);
    struct stat metadata{};
    if (fstat(descriptor, &metadata) || !S_ISREG(metadata.st_mode)) throw std::runtime_error("unsupported");
    std::string text;
    char buffer[4096];
    while (true) {
        const auto count = read(descriptor, buffer, sizeof(buffer));
        if (count == 0) break;
        if (count < 0) throw std::runtime_error("unavailable");
        text.append(buffer, static_cast<std::size_t>(count));
        if (text.size() > 1024 * 1024) throw std::runtime_error("error");
    }
    return text;
}
static void failedField(Json::Value& section, const std::string& field, const std::exception& error)
{
    section["coverage"][field]["status"] = error.what();
    section["coverage"][field]["reason"] = "Native source " + std::string(error.what());
    if (std::string(error.what()) == "permission-denied") section["status"] = "permission-denied";
}
static void localUnavailable(Json::Value& section)
{
    section["status"] = "unavailable";
    for (const auto& name : section["coverage"].getMemberNames()) {
        section["coverage"][name]["status"] = "unavailable";
        section["coverage"][name]["reason"] = "Local data cannot describe a remote Thunder target";
    }
}
void collectPlatform(Json::Value& section, CollectionContext& context)
{
    if (!context.endpoint.local) { localUnavailable(section); return; }
    struct utsname identity{};
    if (uname(&identity) == 0) {
        Json::Value kernel;
        kernel["system"] = identity.sysname;
        kernel["release"] = identity.release;
        kernel["machine"] = identity.machine;
        recordField(section, "kernel", kernel, "uname");
    }
    try {
        std::istringstream input(readBounded(context.procRoot / "uptime"));
        double uptime = 0;
        if (input >> uptime) recordField(section, "uptime", uptime, (context.procRoot / "uptime").string() + " seconds");
    } catch (const std::exception& error) { failedField(section, "uptime", error); }
    try {
        std::istringstream input(readBounded(context.procRoot / "meminfo"));
        Json::Value memory;
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream values(line);
            std::string key, unit;
            Json::UInt64 amount;
            if (values >> key >> amount) {
                values >> unit;
                if (!key.empty() && key.back() == ':') key.pop_back();
                memory[key]["value"] = amount;
                memory[key]["unit"] = unit;
            }
        }
        if (!memory.empty()) recordField(section, "memory", memory, (context.procRoot / "meminfo").string());
    } catch (const std::exception& error) { failedField(section, "memory", error); }
    Json::Value filesystems(Json::arrayValue);
    std::vector<std::string> paths{"/"};
    try {
        std::istringstream mounts(readBounded(context.procRoot / "mounts"));
        std::string line;
        while (std::getline(mounts, line) && paths.size() < context.limit) {
            std::istringstream values(line);
            std::string device, mountpoint, type;
            if (values >> device >> mountpoint >> type) {
                if (mountpoint.find('\\') != std::string::npos) continue;
                if (std::find(paths.begin(), paths.end(), mountpoint) == paths.end()) paths.push_back(mountpoint);
            }
        }
    } catch (...) {}
    for (const auto& path : paths) {
        if (!context.remainingMs()) break;
        struct statvfs filesystem{};
        Json::Value storage;
        storage["path"] = path;
        if (statvfs(path.c_str(), &filesystem) == 0) {
            storage["totalBytes"] = Json::UInt64(filesystem.f_blocks) * filesystem.f_frsize;
            storage["availableBytes"] = Json::UInt64(filesystem.f_bavail) * filesystem.f_frsize;
            storage["status"] = "available";
        } else storage["status"] = "unavailable";
        filesystems.append(storage);
    }
    if (!filesystems.empty()) recordField(section, "filesystems", filesystems, "Native mount inventory and statvfs; bounded to configured limit");
}
void collectConfiguration(Json::Value& section, CollectionContext& context)
{
    if (!context.endpoint.local) { localUnavailable(section); return; }
    Json::Value files(Json::arrayValue);
    for (const auto& path : context.configFiles) {
        if (!context.remainingMs()) break;
        Json::Value entry;
        entry["path"] = path.string();
        entry["interpretation"] = "declared; effective merge policy unverified";
        try {
            const auto extension = path.extension().string();
            if (extension != ".json" && extension != ".yaml" && extension != ".yml")
                throw std::runtime_error("unsupported");
            Json::Value declared;
            if (extension == ".json") declared = parseJson(readBounded(path));
            else {
#ifdef ENTOS_ENABLE_YAML
                std::size_t budget = 2 * 1024 * 1024;
                declared = yamlValue(YAML::Load(readBounded(path)), 0, budget);
#else
                throw std::runtime_error("unsupported");
#endif
            }
            entry["declared"] = sanitize(declared);
            entry["status"] = "ok";
        } catch (...) {
            entry["status"] = "unavailable";
            section["errors"].append("Configuration source unavailable, invalid, or unsupported by this build");
        }
        files.append(entry);
    }
    recordField(section, "files", files, "Ordered JSON configuration sources");
}
void collectInput(Json::Value& section, CollectionContext& context)
{
    if (!context.endpoint.local) { localUnavailable(section); return; }
    try {
        const auto text = readBounded(context.procRoot / "bus/input/devices");
        Json::Value devices(Json::arrayValue);
        std::istringstream lines(text);
        std::string line;
        Json::Value device(Json::objectValue);
        while (std::getline(lines, line)) {
            if (line.empty()) {
                if (!device.empty()) devices.append(device);
                device = Json::Value(Json::objectValue);
            } else if (line.size() > 3 && line[1] == ':') device[line.substr(0, 1)] = line.substr(3);
        }
        if (!device.empty()) devices.append(device);
        recordField(section, "devices", devices, (context.procRoot / "bus/input/devices").string());
    } catch (const std::exception& error) { failedField(section, "devices", error); }
}
}