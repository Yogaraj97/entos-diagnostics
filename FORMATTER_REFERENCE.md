# ENTOS-DIAGNOSTICS FORMATTER REFERENCE GUIDE

## Quick Overview

All 4 formatters in `DumpsysFormatter.cpp` handle conversion from raw JSON API responses to human-readable table format.

### Call Hierarchy
```
CLI Command: entos-diagnostics dumpsys --section [runtime|package|resource|window]
    ↓
main.cpp (parseCommandLine)
    ↓
DiagnosticsCollector (Fetches from Thunder APIs)
    ↓
Snapshot.cpp:textOutput() (Routes to appropriate formatter)
    ↓
DumpsysFormatter::{formatRuntimeManager|formatPackageManager|formatResourceMonitor|formatWindowManager}
    ↓
Console Output (Table Format)
```

---

## 1. RUNTIME MANAGER FORMATTER

**File:** `src/application/DumpsysFormatter.cpp:88-172`

### Function Signature
```cpp
static std::string formatRuntimeManager(const Json::Value& data);
```

### Input JSON Expected
```json
{
  "loaded_apps": [
    {
      "appId": "com.netflix.nrdp",
      "instanceId": "instance-001",
      "state": "running",
      "pid": 1234
    }
  ]
}
```

Alternative field names: `apps` instead of `loaded_apps`

### Output Columns
| Column | Source | Type | Notes |
|--------|--------|------|-------|
| App | `appId` | string | Application identifier |
| Instance ID | `instanceId` | string | Runtime instance |
| State | `state` | string | running, idle, etc. |
| PID | `pid` | int | Process ID |

### Example Output
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

### How to Call
```cpp
#include "entos/DumpsysFormatter.h"

Json::Value data;
data["loaded_apps"][0]["appId"] = "com.netflix.nrdp";
data["loaded_apps"][0]["instanceId"] = "instance-001";
data["loaded_apps"][0]["state"] = "running";
data["loaded_apps"][0]["pid"] = 1234;

std::string formatted = DumpsysFormatter::formatRuntimeManager(data);
std::cout << formatted;
```

### Command Line
```bash
entos-diagnostics dumpsys --section runtime
```

---

## 2. PACKAGE MANAGER FORMATTER

**File:** `src/application/DumpsysFormatter.cpp:173-330`

### Function Signature
```cpp
static std::string formatPackageManager(const Json::Value& data);
```

### Input JSON Expected (Complex Structure)
```json
{
  "downloadThreads": 4,
  "inProgress": [
    {
      "id": "netflix-app",
      "url": "http://repo.example.com/netflix.zip",
      "rateLimit": 1048576,
      "progress": 45
    }
  ],
  "pending": [
    {
      "id": "hulu-app",
      "url": "http://repo.example.com/hulu.zip",
      "rateLimit": 0
    }
  ],
  "platformIds": ["rpi-arm-1.0"],
  "storageDir": "/opt/apps",
  "packages": [
    {
      "app": "com.netflix.nrdp",
      "version": "5.1.2",
      "userId": 0,
      "userName": "root"
    }
  ],
  "cacheEntries": [
    {
      "app": "com.hulu.plus",
      "version": "4.2.1",
      "size": "125 MB",
      "lastUsed": "2025-01-15",
      "path": "/opt/apps/.cache/hulu.zip"
    }
  ],
  "cacheSize": "52 MB",
  "cacheLimit": "100 MB"
}
```

### Output Sections
**1. Installer Section**
- Download Threads (int)
- Requests in Progress (array with progress %)
- Requests Pending (array)

**2. Package Database Section**
- Compatible Platform IDs
- Storage Directory
- Packages Table (App, Version, UserID)

**3. Package Cache Section**
- Cache Size, Cache Limit
- Entries Table (App, Version, Size, Last Used, Format, Path)

### Example Output
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
        Requests Pending: None

    Package Database:
        Compatible Platform Ids: rpi-arm-1.0
        Storage Dir: "/opt/apps"
        RALF Packages Supported: true
        Package Mount Mode: auto
        Packages:
            App                          | Version        | UserID
            ---------------------------- | --------------- | --------
            com.netflix.nrdp             | 5.1.2           | 0 root

    Package Cache:
        Cache Size: 52 MB
        Cache Limit: 100 MB
        Entries:
            App                | Version | Size   | Last Used  | Format | Path
            ----------         | ------- | ------ | ---------- | ------ | ----
            com.hulu.plus      | 4.2.1   | 125 MB | 2025-01-15 | ZIP    | "/opt/apps/.cache/hulu.zip"
```

### How to Call
```cpp
#include "entos/DumpsysFormatter.h"

Json::Value data;
data["downloadThreads"] = 4;
data["packages"][0]["app"] = "com.netflix.nrdp";
data["packages"][0]["version"] = "5.1.2";
// ... more fields

std::string formatted = DumpsysFormatter::formatPackageManager(data);
std::cout << formatted;
```

### Command Line
```bash
entos-diagnostics dumpsys --section package
```

---

## 3. RESOURCE MONITOR FORMATTER

**File:** `src/application/DumpsysFormatter.cpp:332-373`

### Function Signature
```cpp
static std::string formatResourceMonitor(const Json::Value& data);
```

### Input JSON Expected
```json
{
  "stats": {
    "totalCpu": "45%",
    "totalMemory": "512 MB",
    "usedMemory": "256 MB",
    "freeMemory": "256 MB",
    "processes": [
      {
        "pid": 1234,
        "name": "netflix",
        "cpu": "15%",
        "memory": "85 MB"
      },
      {
        "pid": 5678,
        "name": "hulu",
        "cpu": "8%",
        "memory": "120 MB"
      }
    ]
  }
}
```

### Output Sections
**1. System Statistics**
- Total CPU
- Total Memory
- Used Memory
- Free Memory

**2. Process Information (Table)**
- PID, Name, CPU, Memory

### Example Output
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
```

### How to Call
```cpp
#include "entos/DumpsysFormatter.h"

Json::Value data;
data["stats"]["totalCpu"] = "45%";
data["stats"]["processes"][0]["pid"] = 1234;
data["stats"]["processes"][0]["name"] = "netflix";
// ... more fields

std::string formatted = DumpsysFormatter::formatResourceMonitor(data);
std::cout << formatted;
```

### Command Line
```bash
entos-diagnostics dumpsys --section resource
```

---

## 4. WINDOW MANAGER FORMATTER

**File:** `src/application/DumpsysFormatter.cpp:375-406`

### Function Signature
```cpp
static std::string formatWindowManager(const Json::Value& data);
```

### Input JSON Expected
```json
{
  "clients": [
    {
      "app": "com.netflix.nrdp",
      "state": "focused",
      "layer": "1",
      "zorder": "100"
    },
    {
      "app": "com.hulu.plus",
      "state": "background",
      "layer": "2",
      "zorder": "50"
    }
  ],
  "focusedApp": "com.netflix.nrdp"
}
```

Alternative field names: `apps` instead of `clients`

### Output Sections
**1. Window Clients (Table)**
- App, State, Layer, Z-Order

**2. Focused Application**
- Currently focused app

### Example Output
```
WINDOW MANAGER:

    Window Clients:
        App                           | State       | Layer | Z-Order
        ----------------------------- | ----------- | ----- | -------
        com.netflix.nrdp              | focused     | 1     | 100
        com.hulu.plus                 | background  | 2     | 50

    Focused Application: com.netflix.nrdp
```

### How to Call
```cpp
#include "entos/DumpsysFormatter.h"

Json::Value data;
data["clients"][0]["app"] = "com.netflix.nrdp";
data["clients"][0]["state"] = "focused";
data["focusedApp"] = "com.netflix.nrdp";
// ... more fields

std::string formatted = DumpsysFormatter::formatWindowManager(data);
std::cout << formatted;
```

### Command Line
```bash
entos-diagnostics dumpsys --section window
```

---

## CALLING FROM SNAPSHOT.CPP

The main entry point for all formatters is `Snapshot.cpp:textOutput()`:

```cpp
std::string textOutput(const Json::Value& snapshot)
{
    std::ostringstream output;
    for (const auto& section : snapshot["sections"]) {
        std::string sectionId = section["id"].asString();
        
        // Route to appropriate formatter
        if (sectionId == "runtime" && section["data"].isMember("apps")) {
            output << DumpsysFormatter::formatRuntimeManager(section["data"]);
            continue;
        } else if (sectionId == "package" && section["data"].isMember("packages")) {
            output << DumpsysFormatter::formatPackageManager(section["data"]);
            continue;
        } else if (sectionId == "window" && section["data"].isMember("clients")) {
            output << DumpsysFormatter::formatWindowManager(section["data"]);
            continue;
        } else if (sectionId == "resource" && section["data"].isMember("stats")) {
            output << DumpsysFormatter::formatResourceMonitor(section["data"]);
            continue;
        }
        // ... fallback for other sections
    }
    return output.str();
}
```

**Key Points:**
1. Each formatter is called only if the expected data fields are present
2. Formatters receive `section["data"]` JSON object
3. Output is accumulated in `std::ostringstream`
4. Final result sent to console

---

## DATA FIELD MAPPING GUIDE

| Section | Formatter | Input Check | Key Fields |
|---------|-----------|-------------|-----------|
| runtime | formatRuntimeManager | `data.isMember("apps")` | appId, instanceId, state, pid |
| package | formatPackageManager | `data.isMember("packages")` | downloadThreads, packages[], cacheEntries[] |
| resource | formatResourceMonitor | `data.isMember("stats")` | stats.totalCpu, stats.processes[] |
| window | formatWindowManager | `data.isMember("clients")` | clients[], focusedApp |

---

## FIELD NAME VARIANTS SUPPORTED

All formatters handle multiple Thunder API field name variants:

| Data | Variant 1 | Variant 2 |
|------|-----------|-----------|
| Loaded Apps | `loaded_apps` | `apps` |
| Window Clients | `clients` | `apps` |
| Resources CPU | `total_cpu` | `totals.cpu` |
| Resources Memory | `used_bytes` | `used` |

---

## TESTING FORMATTERS

### Unit Test Example
```cpp
#include "entos/DumpsysFormatter.h"
#include <json/json.h>
#include <iostream>

int main() {
    Json::Value runtimeData;
    runtimeData["loaded_apps"][0]["appId"] = "test.app";
    runtimeData["loaded_apps"][0]["instanceId"] = "inst-001";
    runtimeData["loaded_apps"][0]["state"] = "running";
    runtimeData["loaded_apps"][0]["pid"] = 1000;
    
    std::string output = DumpsysFormatter::formatRuntimeManager(runtimeData);
    std::cout << output << std::endl;
    
    return 0;
}
```

### Run All Sections
```bash
entos-diagnostics dumpsys --section runtime --section package --section resource --section window
# or simply
entos-diagnostics dumpsys
```

---

## SUMMARY

✅ **All 4 formatters are production-ready**
- Runtime Manager: Displays loaded apps with instance IDs
- Package Manager: Shows installer, packages, and cache info
- Resource Monitor: Displays system stats and processes
- Window Manager: Shows window clients and focused app

✅ **Key Features:**
- Human-readable table format (NOT JSON)
- Handle multiple field name variants
- Graceful fallbacks for missing data (show `--`)
- Proper column alignment and widths
- Summary sections where appropriate

