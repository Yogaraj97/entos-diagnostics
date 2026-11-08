# AppManager Diagnostics — Version 2.0 (Human-Readable Tables)

**Date**: 2026-11-08  
**Version**: 2.0 (UPDATED: No JSON output, table-based format)  
**Status**: Phase 1 Complete | Ready for Phase 2 Integration

---

## 📋 Overview

AppManager diagnostic output has been redesigned to use **human-readable tables** instead of JSON. This approach provides:

- ✅ Terminal-friendly output (like `dumpsys` or `top`)
- ✅ No JSON parsing needed
- ✅ Clear column layout with alignment
- ✅ Summary statistics below each table
- ✅ Section-based organization
- ✅ Ready for direct copy-paste to documents

---

## 🎯 The 5 AppManager Diagnostic Providers

### 1. **LOADED APPLICATIONS**
```
App ID    | Instance ID  | State    | PID
----------+--------------+----------+-----
netflix   | instance-001 | running  | 1234
youtube   | instance-002 | running  | 5678

    Summary: 2 app(s) loaded
```

### 2. **INSTALLED APPLICATIONS**
```
App ID    | Name        | Version      | Category
----------+-------------+--------------+-------------
netflix   | Netflix     | 100.140.1    | streaming
youtube   | YouTube TV  | 100.2.31     | streaming

    Summary: 2 app(s) installed
```

### 3. **PACKAGE MANAGER**
```
Cache Status:
    Used: 504 MB
    Limit: 100 MB

Package Name | Version    | Size    | Type | Installed
-------------+------------+---------+------+----------
netflix      | 100.140.1  | 52 MB   | zip  | Yes
youtube      | 100.2.31   | 11 MB   | zip  | Yes

    Summary: 2 package(s) in database
```

### 4. **RESOURCE MONITOR**
```
Total Resources:
    CPU: 15.7%
    Memory: 2048 MB

App ID   | CPU (%)  | Memory (MB) | Timestamp
---------+----------+-------------+----------
netflix  | 5.2      | 128         | 1730977661
youtube  | 3.1      | 256         | 1730977661

    Summary: 2 app(s) monitored
```

### 5. **WINDOW MANAGER**
```
Focused Application: netflix

App ID   | Z-Order | Focused | Visible
---------+---------+---------+--------
netflix  | 100     | Yes     | Yes
youtube  | 50      | No      | No

    Summary: 2 window client(s)
```

---

## 📁 Deliverable Files

### C++ Implementation Files

#### Header Files
```
include/entos/AppManagerFormatter.h (1.9 KB)
├─ AppManagerFormatter class
├─ Methods: formatLoadedApps(), formatInstalledApps(), etc.
└─ Table generation utilities

include/entos/AppManagerDumpsys.h (1.5 KB)
├─ AppManagerDumpsys class  
├─ Methods: getLoadedApps(), getInstalledApps(), etc.
└─ Integration with dumpsys system
```

#### Implementation Files
```
src/application/AppManagerFormatter.cpp (12.5 KB)
├─ Table formatting logic
├─ Column alignment functions
├─ Summary calculation
└─ Row/header/separator generation
```

### Documentation Files

```
APPMANAGER_OUTPUT_FORMAT.md (11.3 KB)
├─ Complete format specification
├─ Example output for each provider
├─ Column descriptions
└─ Usage examples

OPENSPEC_CONSOLIDATED.yaml (9.5 KB)
├─ Spec-driven design document
├─ Provider contracts
├─ Implementation phases
└─ Verification checklist
```

### Configuration

```
config/providers.json (UPDATED)
├─ 5 AppManager providers
├─ Field mappings (13 total)
├─ Enabled: false (verification gate)
└─ Verification: provisional
```

---

## 🚀 Quick Start

### 1. Review the Output Format
```bash
cat APPMANAGER_OUTPUT_FORMAT.md
```

### 2. Review the Implementation
```bash
cat include/entos/AppManagerFormatter.h
cat src/application/AppManagerFormatter.cpp
```

### 3. Review the Specification
```bash
cat openspec/changes/add-appmanager-diagnostics/OPENSPEC_CONSOLIDATED.yaml
```

### 4. Test with Mock Data
```bash
# Mock data available in:
ls tests/mocks/app_*.json
```

---

## 🔄 Implementation Phases

### Phase 1: Format Design ✅ **COMPLETE**

- [x] Defined table output format
- [x] Created AppManagerFormatter class (12.5 KB)
- [x] Created AppManagerDumpsys header
- [x] Documented all 5 providers
- [x] Created output format specification
- [x] Created consolidated OpenSpec

### Phase 2: Integration 🔄 **READY**

**What needs to be done:**
- [ ] Implement AppManagerDumpsys.cpp
- [ ] Integrate with main.cpp
- [ ] Add CLI argument parsing
- [ ] Connect formatters to output stream

### Phase 3: Unit Testing 🔄 **READY**

**Test using mock data from tests/mocks/**

### Phase 4: Device Verification 🚫 **BLOCKED**

Awaits device access

---

## 📊 Architecture

```
entos-diagnostics dumpsys --section runtime
          ↓
CollectionEngine.collect()
          ↓
AppCollector.collectApplication()
          ↓
RpcClient.call() → Thunder plugins
          ↓
Parse JSON responses
          ↓
AppManagerFormatter.format*()
          ↓
Human-readable table
          ↓
Terminal output
```

---

## ✅ Verification Checklist

- [x] AppManagerFormatter.h created
- [x] AppManagerFormatter.cpp created (12.5 KB)
- [x] AppManagerDumpsys.h created
- [x] APPMANAGER_OUTPUT_FORMAT.md created
- [x] OPENSPEC_CONSOLIDATED.yaml created
- [ ] AppManagerDumpsys.cpp implementation
- [ ] main.cpp integration
- [ ] Unit tests with mock data
- [ ] Device verification

---

**Status**: ✅ Phase 1 Complete | Ready for Phase 2  
**Created**: 2026-11-08  
**Version**: 2.0 (Human-Readable Tables)
