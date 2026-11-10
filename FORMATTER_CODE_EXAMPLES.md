# FORMATTER CODE EXAMPLES & QUICK START

## Quick Reference: All 4 Formatters

### 1. Runtime Manager - Load Loaded Apps

**What It Does:** Displays all currently loaded applications

**How to Call:**
```bash
entos-diagnostics dumpsys --section runtime
```

**Code Example:**
```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

int main() {
    // Create sample runtime data
    Json::Value runtimeData;
    
    // Add multiple loaded apps
    runtimeData["loaded_apps"][0]["appId"] = "com.netflix.nrdp";
    runtimeData["loaded_apps"][0]["instanceId"] = "instance-001";
    runtimeData["loaded_apps"][0]["state"] = "running";
    runtimeData["loaded_apps"][0]["pid"] = 1234;
    
    runtimeData["loaded_apps"][1]["appId"] = "com.bskyb.epgui";
    runtimeData["loaded_apps"][1]["instanceId"] = "instance-002";
    runtimeData["loaded_apps"][1]["state"] = "running";
    runtimeData["loaded_apps"][1]["pid"] = 5678;
    
    // Call formatter
    std::string output = DumpsysFormatter::formatRuntimeManager(runtimeData);
    
    // Print to console
    std::cout << output << std::endl;
    
    return 0;
}
```

**Expected Output:**
```
RUNTIME MANAGER:

    GUIAppManagerV2:
        App                  | Instance ID      | State                | PID
        -------------------- | ---------------- | -------------------- | --------------------
        com.netflix.nrdp     | instance-001     | running              | 1234
        com.bskyb.epgui      | instance-002     | running              | 5678

    Summary:
        Loaded Apps: 2
```

**Required Fields in JSON:**
- `loaded_apps[]` OR `apps[]` (array)
  - `appId` (string) - Application identifier
  - `instanceId` (string) - Instance identifier
  - `state` (string) - Application state
  - `pid` (integer) - Process ID

---

### 2. Package Manager - Show Packages & Cache

**What It Does:** Displays installer info, packages, and cache entries

**How to Call:**
```bash
entos-diagnostics dumpsys --section package
```

**Code Example:**
```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

int main() {
    Json::Value packageData;
    
    // Installer section
    packageData["downloadThreads"] = 4;
    
    // In-progress requests
    packageData["inProgress"][0]["id"] = "netflix-app";
    packageData["inProgress"][0]["url"] = "http://repo.example.com/netflix.zip";
    packageData["inProgress"][0]["rateLimit"] = 1048576;
    packageData["inProgress"][0]["progress"] = 45;
    
    // Pending requests
    packageData["pending"][0]["id"] = "hulu-app";
    packageData["pending"][0]["url"] = "http://repo.example.com/hulu.zip";
    packageData["pending"][0]["rateLimit"] = 0;
    
    // Package database
    packageData["platformIds"].append("rpi-arm-1.0");
    packageData["platformIds"].append("rpi-arm-2.0");
    packageData["storageDir"] = "/opt/apps";
    
    // Packages array
    packageData["packages"][0]["app"] = "com.netflix.nrdp";
    packageData["packages"][0]["version"] = "5.1.2";
    packageData["packages"][0]["userId"] = 0;
    packageData["packages"][0]["userName"] = "root";
    
    packageData["packages"][1]["app"] = "com.hulu.plus";
    packageData["packages"][1]["version"] = "4.2.1";
    packageData["packages"][1]["userId"] = 0;
    packageData["packages"][1]["userName"] = "root";
    
    // Cache info
    packageData["cacheSize"] = "52 MB";
    packageData["cacheLimit"] = "100 MB";
    
    // Cache entries
    packageData["cacheEntries"][0]["app"] = "com.disney.plus";
    packageData["cacheEntries"][0]["version"] = "3.1.0";
    packageData["cacheEntries"][0]["size"] = "85 MB";
    packageData["cacheEntries"][0]["lastUsed"] = "2025-01-15";
    packageData["cacheEntries"][0]["path"] = "/opt/apps/.cache/disney.zip";
    
    // Format and print
    std::string output = DumpsysFormatter::formatPackageManager(packageData);
    std::cout << output << std::endl;
    
    return 0;
}
```

**Expected Output (Partial):**
```
PACKAGE MANAGER:

    Installer:
        Download Threads: 4
        Requests in Progress:
            netflix-app -
              Url: http://repo.example.com/netflix.zip
              Status: Downloading
              RateLimit: 1048576 bps
              Progress: 45%
        Requests Pending:
            hulu-app -
              Url: http://repo.example.com/hulu.zip
              Status: Pending
              RateLimit: none

    Package Database:
        Compatible Platform Ids: rpi-arm-1.0, rpi-arm-2.0
        Storage Dir: "/opt/apps"
        RALF Packages Supported: true
        Package Mount Mode: auto
        Packages:
            App                          | Version        | UserID
            ----------------------       | --------------- | --------
            com.netflix.nrdp             | 5.1.2           | 0 root
            com.hulu.plus                | 4.2.1           | 0 root

    Package Cache:
        Cache Size: 52 MB
        Cache Limit: 100 MB
        Entries:
            App                | Version | Size   | Last Used | Format | Path
            --------           | ------- | ------ | --------- | ------ | ----
            com.disney.plus    | 3.1.0   | 85 MB  | 2025-01-15| ZIP    | "/opt/apps/.cache/disney.zip"
```

**Required Fields in JSON:**
- `downloadThreads` (integer)
- `inProgress[]` (array of request objects)
  - `id`, `url`, `rateLimit`, `progress`
- `pending[]` (array of request objects)
  - `id`, `url`, `rateLimit`
- `platformIds[]` (string array)
- `storageDir` (string)
- `packages[]` (array of package objects)
  - `app`, `version`, `userId`, `userName`
- `cacheSize`, `cacheLimit` (strings)
- `cacheEntries[]` (array of cache entry objects)
  - `app`, `version`, `size`, `lastUsed`, `path`

---

### 3. Resource Monitor - Show System Stats

**What It Does:** Displays system CPU/memory and per-process stats

**How to Call:**
```bash
entos-diagnostics dumpsys --section resource
```

**Code Example:**
```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

int main() {
    Json::Value resourceData;
    
    // System statistics
    resourceData["stats"]["totalCpu"] = "45%";
    resourceData["stats"]["totalMemory"] = "512 MB";
    resourceData["stats"]["usedMemory"] = "256 MB";
    resourceData["stats"]["freeMemory"] = "256 MB";
    
    // Process list
    resourceData["stats"]["processes"][0]["pid"] = 1234;
    resourceData["stats"]["processes"][0]["name"] = "netflix";
    resourceData["stats"]["processes"][0]["cpu"] = "15%";
    resourceData["stats"]["processes"][0]["memory"] = "85 MB";
    
    resourceData["stats"]["processes"][1]["pid"] = 5678;
    resourceData["stats"]["processes"][1]["name"] = "hulu";
    resourceData["stats"]["processes"][1]["cpu"] = "8%";
    resourceData["stats"]["processes"][1]["memory"] = "120 MB";
    
    resourceData["stats"]["processes"][2]["pid"] = 9999;
    resourceData["stats"]["processes"][2]["name"] = "system";
    resourceData["stats"]["processes"][2]["cpu"] = "22%";
    resourceData["stats"]["processes"][2]["memory"] = "51 MB";
    
    // Format and print
    std::string output = DumpsysFormatter::formatResourceMonitor(resourceData);
    std::cout << output << std::endl;
    
    return 0;
}
```

**Expected Output:**
```
RESOURCE MONITOR:

    System Statistics:
        Total CPU: 45%
        Total Memory: 512 MB
        Used Memory: 256 MB
        Free Memory: 256 MB

    Process Information:
        PID    | Name                          | CPU     | Memory
        ------ | ----------------------------- | ------- | --------
        1234   | netflix                       | 15%     | 85 MB
        5678   | hulu                          | 8%      | 120 MB
        9999   | system                        | 22%     | 51 MB
```

**Required Fields in JSON:**
- `stats` (object)
  - `totalCpu` (string or numeric)
  - `totalMemory` (string)
  - `usedMemory` (string)
  - `freeMemory` (string)
  - `processes[]` (array of process objects)
    - `pid` (integer)
    - `name` (string)
    - `cpu` (string)
    - `memory` (string)

---

### 4. Window Manager - Show Window Clients

**What It Does:** Displays window clients and focused application

**How to Call:**
```bash
entos-diagnostics dumpsys --section window
```

**Code Example:**
```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

int main() {
    Json::Value windowData;
    
    // Window clients
    windowData["clients"][0]["app"] = "com.netflix.nrdp";
    windowData["clients"][0]["state"] = "focused";
    windowData["clients"][0]["layer"] = "1";
    windowData["clients"][0]["zorder"] = "100";
    
    windowData["clients"][1]["app"] = "com.hulu.plus";
    windowData["clients"][1]["state"] = "background";
    windowData["clients"][1]["layer"] = "2";
    windowData["clients"][1]["zorder"] = "50";
    
    windowData["clients"][2]["app"] = "com.disney.plus";
    windowData["clients"][2]["state"] = "background";
    windowData["clients"][2]["layer"] = "3";
    windowData["clients"][2]["zorder"] = "25";
    
    // Focused application
    windowData["focusedApp"] = "com.netflix.nrdp";
    
    // Format and print
    std::string output = DumpsysFormatter::formatWindowManager(windowData);
    std::cout << output << std::endl;
    
    return 0;
}
```

**Expected Output:**
```
WINDOW MANAGER:

    Window Clients:
        App                           | State       | Layer | Z-Order
        ----------------------------- | ----------- | ----- | -------
        com.netflix.nrdp              | focused     | 1     | 100
        com.hulu.plus                 | background  | 2     | 50
        com.disney.plus               | background  | 3     | 25

    Focused Application: com.netflix.nrdp
```

**Required Fields in JSON:**
- `clients[]` (array of window client objects)
  - `app` (string)
  - `state` (string)
  - `layer` (string)
  - `zorder` (string)
- `focusedApp` (string)

---

## Integration Example: Building Complete Dumpsys Output

```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

// Helper function to build sample data
Json::Value buildSampleSnapshot() {
    Json::Value sections;
    
    // Runtime section
    sections["sections"][0]["id"] = "runtime";
    sections["sections"][0]["title"] = "Runtime Manager";
    sections["sections"][0]["status"] = "ok";
    sections["sections"][0]["data"]["loaded_apps"][0]["appId"] = "com.netflix.nrdp";
    sections["sections"][0]["data"]["loaded_apps"][0]["instanceId"] = "inst-001";
    sections["sections"][0]["data"]["loaded_apps"][0]["state"] = "running";
    sections["sections"][0]["data"]["loaded_apps"][0]["pid"] = 1234;
    
    // Package section
    sections["sections"][1]["id"] = "package";
    sections["sections"][1]["title"] = "Package Manager";
    sections["sections"][1]["status"] = "ok";
    sections["sections"][1]["data"]["downloadThreads"] = 4;
    sections["sections"][1]["data"]["packages"][0]["app"] = "com.netflix.nrdp";
    sections["sections"][1]["data"]["packages"][0]["version"] = "5.1.2";
    sections["sections"][1]["data"]["packages"][0]["userId"] = 0;
    sections["sections"][1]["data"]["cacheSize"] = "52 MB";
    sections["sections"][1]["data"]["cacheLimit"] = "100 MB";
    
    // Resource section
    sections["sections"][2]["id"] = "resource";
    sections["sections"][2]["title"] = "Resource Monitor";
    sections["sections"][2]["status"] = "ok";
    sections["sections"][2]["data"]["stats"]["totalCpu"] = "45%";
    sections["sections"][2]["data"]["stats"]["totalMemory"] = "512 MB";
    
    // Window section
    sections["sections"][3]["id"] = "window";
    sections["sections"][3]["title"] = "Window Manager";
    sections["sections"][3]["status"] = "ok";
    sections["sections"][3]["data"]["clients"][0]["app"] = "com.netflix.nrdp";
    sections["sections"][3]["data"]["clients"][0]["state"] = "focused";
    sections["sections"][3]["data"]["focusedApp"] = "com.netflix.nrdp";
    
    return sections;
}

int main() {
    // Build sample data
    Json::Value snapshot = buildSampleSnapshot();
    
    // Print header
    std::cout << "===========================================\n";
    std::cout << "       ENTOS-DIAGNOSTICS DUMPSYS OUTPUT\n";
    std::cout << "===========================================\n";
    
    // Process each section
    for (const auto& section : snapshot["sections"]) {
        std::string sectionId = section["id"].asString();
        
        if (sectionId == "runtime" && section["data"].isMember("loaded_apps")) {
            std::cout << DumpsysFormatter::formatRuntimeManager(section["data"]);
        } else if (sectionId == "package" && section["data"].isMember("packages")) {
            std::cout << DumpsysFormatter::formatPackageManager(section["data"]);
        } else if (sectionId == "resource" && section["data"].isMember("stats")) {
            std::cout << DumpsysFormatter::formatResourceMonitor(section["data"]);
        } else if (sectionId == "window" && section["data"].isMember("clients")) {
            std::cout << DumpsysFormatter::formatWindowManager(section["data"]);
        }
    }
    
    return 0;
}
```

---

## Testing Each Formatter in Isolation

### Runtime Manager Test
```bash
# Direct binary test (if you have a test app)
./build/test_formatters runtime

# Or via CLI
entos-diagnostics dumpsys --section runtime
```

### Package Manager Test
```bash
entos-diagnostics dumpsys --section package
```

### Resource Monitor Test
```bash
entos-diagnostics dumpsys --section resource
```

### Window Manager Test
```bash
entos-diagnostics dumpsys --section window
```

### All Sections Test
```bash
entos-diagnostics dumpsys --section runtime --section package --section resource --section window
# or simply
entos-diagnostics dumpsys
```

---

## Common Issues & Solutions

### Issue: Formatter shows `--` for all fields

**Cause:** Required field names don't exist in JSON

**Solution:** Check field names match expected format:
- Runtime: Use `loaded_apps` or `apps`
- Window: Use `clients` or `apps`
- Resource: Use `stats`
- Package: Use `packages`

### Issue: Table columns misaligned

**Cause:** Long values or missing padding

**Solution:** Formatter automatically pads columns to minimum width (20 chars)

### Issue: No output from formatter

**Cause:** Section data not matching expected type

**Solution:** Verify in code:
```cpp
if (section["data"].isMember("expectedField")) {
    // Safe to call formatter
}
```

---

## Performance Notes

- **Runtime Manager:** O(n) where n = number of loaded apps (typically < 100)
- **Package Manager:** O(n) where n = number of packages (typically < 1000)
- **Resource Monitor:** O(n) where n = number of processes (typically < 500)
- **Window Manager:** O(n) where n = number of window clients (typically < 50)

All formatters build strings in-memory using `std::ostringstream`. No external I/O except final console output.

