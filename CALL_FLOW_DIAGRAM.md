# ENTOS-DIAGNOSTICS: COMPLETE CALL FLOW DIAGRAM

## End-to-End Flow: CLI to Table Output

```
┌────────────────────────────────────────────────────────────────────────────┐
│                          USER COMMAND LINE                                  │
│  $ entos-diagnostics dumpsys --section runtime --section package            │
└────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ FILE: src/cli/main.cpp → main()                                             │
│                                                                              │
│ Steps:                                                                       │
│  1. Parse command line arguments                                            │
│  2. Extract "--section runtime" and "--section package"                     │
│  3. Create DiagnosticsCollector with selected sections                      │
│  4. Create Snapshot object                                                  │
└────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ CLASS: DiagnosticsCollector                                                 │
│ FILE: src/core/DiagnosticsCollector.cpp                                     │
│                                                                              │
│ For each selected section, collect data via Thunder APIs:                   │
│                                                                              │
│ ┌─ Section: "runtime" ──────────────────────────────────────────────────┐  │
│ │                                                                        │  │
│ │ Method: collectRuntimeData()                                           │  │
│ │ Thunder API Call:                                                      │  │
│ │   org.rdk.AppManager.getLoadedApps()                                   │  │
│ │                                                                        │  │
│ │ API Response:                                                          │  │
│ │ {                                                                      │  │
│ │   "result": {                                                          │  │
│ │     "apps": [                                                          │  │
│ │       {                                                                │  │
│ │         "appId": "com.netflix.nrdp",                                   │  │
│ │         "instanceId": "instance-001",                                  │  │
│ │         "state": "running",                                            │  │
│ │         "pid": 1234                                                    │  │
│ │       },                                                               │  │
│ │       ...                                                              │  │
│ │     ]                                                                  │  │
│ │   }                                                                    │  │
│ │ }                                                                      │  │
│ │                                                                        │  │
│ │ Return: Json::Value with structure: {"loaded_apps": [...]}            │  │
│ │                                                                        │  │
│ └────────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│ ┌─ Section: "package" ──────────────────────────────────────────────────┐  │
│ │                                                                        │  │
│ │ Method: collectPackageData()                                           │  │
│ │ Thunder API Call:                                                      │  │
│ │   org.rdk.AppPackageManager.listPackages()                             │  │
│ │                                                                        │  │
│ │ API Response:                                                          │  │
│ │ {                                                                      │  │
│ │   "result": {                                                          │  │
│ │     "packages": [...],                                                 │  │
│ │     "cache": {...},                                                    │  │
│ │     "cacheEntries": [...]                                              │  │
│ │   }                                                                    │  │
│ │ }                                                                      │  │
│ │                                                                        │  │
│ │ Return: Json::Value with package data                                  │  │
│ │                                                                        │  │
│ └────────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│ [Similar for "resource" and "window" sections...]                           │
│                                                                              │
│ FINAL OUTPUT:                                                                │
│ Snapshot object containing all collected section data                       │
└────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ FILE: src/core/Snapshot.cpp → textOutput()                                  │
│                                                                              │
│ PURPOSE: Route each section to appropriate formatter                        │
│                                                                              │
│ For each section in snapshot["sections"]:                                   │
│                                                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │ Get section["id"] and section["data"]                               │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                     │                                                       │
│                     ▼                                                       │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │ if (sectionId == "runtime" && data.isMember("apps"))                │  │
│   │   CALL: formatRuntimeManager(data)  ──────┐                         │  │
│   │                                           │                         │  │
│   │ else if (sectionId == "package" && data.isMember("packages"))       │  │
│   │   CALL: formatPackageManager(data)    ────┤                         │  │
│   │                                           │                         │  │
│   │ else if (sectionId == "resource" && data.isMember("stats"))         │  │
│   │   CALL: formatResourceMonitor(data)   ────┤                         │  │
│   │                                           │                         │  │
│   │ else if (sectionId == "window" && data.isMember("clients"))         │  │
│   │   CALL: formatWindowManager(data)     ────┤                         │  │
│   │                                           │                         │  │
│   │ else                                      │                         │  │
│   │   Use default JSON formatting         ────┘                         │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│ Accumulate all formatted output in std::ostringstream                       │
└────────────────────────────────────────────────────────────────────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    ▼                 ▼                 ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ FILE: src/application/DumpsysFormatter.cpp                                   │
│                                                                              │
│ ┌──────────────── FORMATTER 1 ────────────────┐                             │
│ │ formatRuntimeManager(Json::Value& data)     │                             │
│ │ Lines: 88-172                               │                             │
│ │                                             │                             │
│ │ Input JSON:                                 │                             │
│ │ {                                           │                             │
│ │   "loaded_apps": [                          │                             │
│ │     {                                       │                             │
│ │       "appId": "com.netflix.nrdp",          │                             │
│ │       "instanceId": "instance-001",         │                             │
│ │       "state": "running",                   │                             │
│ │       "pid": 1234                           │                             │
│ │     }                                       │                             │
│ │   ]                                         │                             │
│ │ }                                           │                             │
│ │                                             │                             │
│ │ Processing:                                 │                             │
│ │  1. Check if "loaded_apps" or "apps"       │                             │
│ │  2. Create table header: App | Instance ID  │                             │
│ │  3. For each app, extract fields            │                             │
│ │  4. Format as table row with padding        │                             │
│ │  5. Add summary line                        │                             │
│ │                                             │                             │
│ │ Output: String (human-readable table)       │                             │
│ └──────────────────────────────────────────────┘                             │
│                                                                              │
│ ┌──────────────── FORMATTER 2 ────────────────┐                             │
│ │ formatPackageManager(Json::Value& data)     │                             │
│ │ Lines: 173-330                              │                             │
│ │                                             │                             │
│ │ Sections:                                   │                             │
│ │  • Installer (threads, in-progress, pending)│                             │
│ │  • Package Database (platforms, packages)   │                             │
│ │  • Package Cache (entries)                  │                             │
│ │                                             │                             │
│ │ Output: Multi-section formatted string      │                             │
│ └──────────────────────────────────────────────┘                             │
│                                                                              │
│ ┌──────────────── FORMATTER 3 ────────────────┐                             │
│ │ formatResourceMonitor(Json::Value& data)    │                             │
│ │ Lines: 332-373                              │                             │
│ │                                             │                             │
│ │ Sections:                                   │                             │
│ │  • System Statistics (CPU, Memory)          │                             │
│ │  • Process Information (table)              │                             │
│ │                                             │                             │
│ │ Output: Multi-section formatted string      │                             │
│ └──────────────────────────────────────────────┘                             │
│                                                                              │
│ ┌──────────────── FORMATTER 4 ────────────────┐                             │
│ │ formatWindowManager(Json::Value& data)      │                             │
│ │ Lines: 375-406                              │                             │
│ │                                             │                             │
│ │ Sections:                                   │                             │
│ │  • Window Clients (table)                   │                             │
│ │  • Focused Application                      │                             │
│ │                                             │                             │
│ │ Output: Multi-section formatted string      │                             │
│ └──────────────────────────────────────────────┘                             │
└─────────────────────────────────────────────────────────────────────────────┘
                    │                 │                 │
                    └─────────────────┼─────────────────┘
                                      │
                    ┌─────────────────┴──────────────────┐
                    │                                    │
                    ▼                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ CONSOLE OUTPUT: Human-Readable Table Format                                │
│                                                                             │
│ ===========================================                                 │
│        ENTOS-DIAGNOSTICS DUMPSYS OUTPUT                                    │
│ ===========================================                                 │
│                                                                             │
│ RUNTIME MANAGER:                                                           │
│                                                                             │
│     GUIAppManagerV2:                                                        │
│         App                  | Instance ID      | State  | PID              │
│         -------------------- | ---------------- | ------ | ----------       │
│         com.netflix.nrdp     | instance-001     | running| 1234             │
│         com.bskyb.epgui      | instance-002     | running| 5678             │
│                                                                             │
│     Summary:                                                                │
│         Loaded Apps: 2                                                      │
│                                                                             │
│ PACKAGE MANAGER:                                                           │
│                                                                             │
│     Installer:                                                              │
│         Download Threads: 4                                                 │
│         Requests in Progress: None                                          │
│         Requests Pending: None                                              │
│                                                                             │
│     Package Database:                                                       │
│         Compatible Platform Ids: rpi-arm-1.0                               │
│         Storage Dir: "/opt/apps"                                            │
│         RALF Packages Supported: true                                       │
│         Package Mount Mode: auto                                            │
│         Packages:                                                            │
│             App                          | Version | UserID                │
│             ----------------------       | ------- | ---------             │
│             com.netflix.nrdp             | 5.1.2   | 0 root                │
│                                                                             │
│     Package Cache:                                                          │
│         Cache Size: 52 MB                                                   │
│         Cache Limit: 100 MB                                                 │
│         Entries:                                                             │
│             App              | Version | Size   | Last Used | Format | Path
│             --------         | ------- | -----  | --------- | ------ | ----
│             com.hulu.plus    | 4.2.1   | 125 MB | 2025-01-15| ZIP    | "/opt/apps/.cache/hu
│                                                                             │
└────────────────────────────────────────────────────────────────────────────┘
```

---

## Detailed Data Flow for Each Formatter

### RUNTIME MANAGER FLOW

```
Thunder API Request
└─ org.rdk.AppManager.getLoadedApps()
   └─ Response: {"apps": [{appId, instanceId, state, pid}, ...]}
      └─ DiagnosticsCollector.collectRuntimeData()
         └─ Returns: Json::Value with "loaded_apps" array
            └─ Snapshot.textOutput()
               └─ Checks: section["id"] == "runtime" && section["data"].isMember("apps")
                  └─ Calls: DumpsysFormatter::formatRuntimeManager(section["data"])
                     └─ Parse "loaded_apps" or "apps" array
                        └─ For each app entry:
                           - Extract: appId, instanceId, state, pid
                           - Build table row with padding
                        └─ Create header and separator rows
                        └─ Add summary with total count
                           └─ Return formatted string
                              └─ Console Output
```

### PACKAGE MANAGER FLOW

```
Multiple Thunder API Requests
├─ org.rdk.AppPackageManager.listPackages()
├─ org.rdk.AppPackageManager.getDownloadThreads()
├─ org.rdk.AppPackageManager.getInProgress()
├─ org.rdk.AppPackageManager.getPending()
└─ org.rdk.AppPackageManager.getCacheInfo()
   └─ Responses merged: {packages, cacheEntries, etc.}
      └─ DiagnosticsCollector.collectPackageData()
         └─ Returns: Json::Value with all package fields
            └─ Snapshot.textOutput()
               └─ Checks: section["id"] == "package" && section["data"].isMember("packages")
                  └─ Calls: DumpsysFormatter::formatPackageManager(section["data"])
                     └─ Parse Installer section:
                        ├─ Get downloadThreads
                        ├─ Get inProgress array
                        └─ Get pending array
                     └─ Parse Package Database section:
                        ├─ Get platformIds
                        ├─ Get storageDir
                        └─ Get packages array → build table
                     └─ Parse Cache section:
                        ├─ Get cacheSize, cacheLimit
                        └─ Get cacheEntries → build table
                           └─ Return formatted string
                              └─ Console Output
```

### RESOURCE MONITOR FLOW

```
Thunder API Request
└─ org.rdk.ResourceMonitor.getStats()
   └─ Response: {stats: {totalCpu, totalMemory, processes[]}}
      └─ DiagnosticsCollector.collectResourceData()
         └─ Returns: Json::Value with stats
            └─ Snapshot.textOutput()
               └─ Checks: section["id"] == "resource" && section["data"].isMember("stats")
                  └─ Calls: DumpsysFormatter::formatResourceMonitor(section["data"])
                     └─ Extract System Statistics:
                        ├─ totalCpu
                        ├─ totalMemory
                        ├─ usedMemory
                        └─ freeMemory
                     └─ Extract Process Information:
                        ├─ For each process in stats.processes:
                        │  - Extract: pid, name, cpu, memory
                        │  - Build table row
                        └─ Create header and rows
                           └─ Return formatted string
                              └─ Console Output
```

### WINDOW MANAGER FLOW

```
Thunder API Requests
├─ org.rdk.WindowManager.getClients()
└─ org.rdk.WindowManager.getFocused()
   └─ Responses: {clients: [{app, state, layer, zorder}], focusedApp: "..."}
      └─ DiagnosticsCollector.collectWindowData()
         └─ Returns: Json::Value with window data
            └─ Snapshot.textOutput()
               └─ Checks: section["id"] == "window" && section["data"].isMember("clients")
                  └─ Calls: DumpsysFormatter::formatWindowManager(section["data"])
                     └─ Parse Window Clients:
                        ├─ For each client in clients array:
                        │  - Extract: app, state, layer, zorder
                        │  - Build table row
                        └─ Create header and rows
                     └─ Get focusedApp
                        └─ Return formatted string
                           └─ Console Output
```

---

## Key Data Transformations

| Step | Input Type | Output Type | Example |
|------|-----------|------------|---------|
| Thunder API | HTTP JSON-RPC | Json::Value | `{"result": {...}}` |
| DiagnosticsCollector | Json::Value | Json::Value | Extracts result field |
| Snapshot | Json::Value | Json::Value | Organizes into sections |
| DumpsysFormatter | Json::Value | std::string | Human-readable table |
| Console | std::string | Text Output | Displayed to user |

---

## Field Name Variant Handling

Each formatter checks multiple field name variants in priority order:

### Runtime Manager
```cpp
// Check both names in order
if (data.isMember("loaded_apps") && data["loaded_apps"].isArray()) {
    apps_array = data["loaded_apps"];
} else if (data.isMember("apps") && data["apps"].isArray()) {
    apps_array = data["apps"];
}
```

### Window Manager
```cpp
// Similar pattern for window clients
if (data.isMember("clients") && data["clients"].isArray()) {
    // use clients
} else if (data.isMember("apps") && data["apps"].isArray()) {
    // use apps as fallback
}
```

### Resource Monitor
```cpp
// Handle both field name variants for CPU
std::string cpu = stats.get("total_cpu", "--").asString();
if (cpu == "--") {
    cpu = stats.get("totals.cpu", "--").asString();
}
```

---

## Error Handling Strategy

All formatters use defensive programming:

```cpp
// 1. Check if field exists before accessing
if (data.isMember("fieldName")) {
    // Use field
} else {
    // Use default "--"
}

// 2. Check if array before iterating
if (array.isArray()) {
    for (const auto& item : array) {
        // Process item
    }
} else {
    // Show "no data" message
}

// 3. Handle type conversions safely
if (value.isInt()) {
    // Convert int to string
} else if (value.isString()) {
    // Already string
}
```

