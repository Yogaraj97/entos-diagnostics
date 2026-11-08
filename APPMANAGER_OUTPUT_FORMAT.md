# AppManager Diagnostics — Output Format & Implementation

**Date**: 2026-11-08  
**Version**: 2.0 (Human-Readable Tables)  
**Status**: Updated with simplified format (no JSON output)

---

## Overview

AppManager diagnostic output is now formatted as **human-readable tables** instead of JSON. This provides:
- ✅ Clean, scannable format (like `dumpsys` or `top` command)
- ✅ No JSON parsing needed (direct terminal viewing)
- ✅ Proper column alignment and separators
- ✅ Summary statistics below each table
- ✅ Section-based organization (LOADED APPLICATIONS, PACKAGE MANAGER, etc.)

---

## Output Format Examples

### 1. LOADED APPLICATIONS

```
LOADED APPLICATIONS:
===================

App ID                  | Instance ID         | State            | PID
-----------------------+-----------------------+------------------+-------
com.netflix.nrdp        | instance-001        | running          | 1234
com.bskyb.epgui         | instance-002        | running          | 5678

    Summary: 2 app(s) loaded
```

**Fields:**
- **App ID**: Application identifier
- **Instance ID**: Unique instance identifier
- **State**: Application state (running, paused, stopped, etc.)
- **PID**: Process ID (if available)

---

### 2. INSTALLED APPLICATIONS

```
INSTALLED APPLICATIONS:
======================

App ID                  | Name                | Version          | Category
-----------------------+---------------------+------------------+----------------
com.netflix.nrdp        | Netflix             | 100.140.1.22110  | entertainment
com.bskyb.epgui         | Sky EPG UI          | 4.22.2           | system
youtube_youtube_tv      | YouTube TV          | 100.2.31         | entertainment

    Summary: 3 app(s) installed
```

**Fields:**
- **App ID**: Application package identifier
- **Name**: Human-readable application name
- **Version**: Installed version
- **Category**: Application category/type

---

### 3. PACKAGE MANAGER

```
PACKAGE MANAGER:
===============

    Cache Status:
        Used: 504 MB
        Limit: 100 MB

    Packages:

Package Name            | Version          | Size         | Type    | Installed
------------------------+------------------+--------------+---------+----------
com.netflix.nrdp        | 100.140.1.22110  | 52 MB        | zip     | Yes
com.bskyb.epgui         | 4.22.2           | 11 MB        | zip     | Yes
xumo                    | 100.100.1.1110   | 480 kB       | zip     | Yes
com.sky.rdkbrowser      | 3.1.2            | 11 MB        | zip     | Yes

    Summary: 4 package(s) in database
```

**Fields:**
- **Package Name**: Package identifier
- **Version**: Package version
- **Size**: Installed size
- **Type**: Package type (zip, ralf, etc.)
- **Installed**: Installation status (Yes/No)

**Cache Info:**
- **Used**: Currently used cache space (MB)
- **Limit**: Maximum cache capacity (MB)

---

### 4. RESOURCE MONITOR

```
RESOURCE MONITOR:
===============

    Total Resources:
        CPU: 15.7%
        Memory: 2048 MB

    Per-Application Stats:

App ID                  | CPU (%)          | Memory (MB)    | Timestamp
-----------------------+------------------+----------------+---------------
com.netflix.nrdp        | 5.2              | 128            | 1730977661
com.bskyb.epgui         | 3.1              | 256            | 1730977661
youtube_youtube_tv      | 2.4              | 192            | 1730977661
com.sky.rdkbrowser      | 4.8              | 512            | 1730977661

    Summary: 4 app(s) monitored
```

**Total Resources:**
- **CPU**: Aggregate CPU usage across all apps (%)
- **Memory**: Total memory used by applications (MB)

**Per-App Stats:**
- **App ID**: Application identifier
- **CPU (%)**: CPU usage percentage
- **Memory (MB)**: Memory consumption in MB
- **Timestamp**: Server-side timestamp (Unix epoch)

---

### 5. WINDOW MANAGER

```
WINDOW MANAGER:
===============

    Focused Application: com.bskyb.epgui

    Window Clients:

App ID                  | Z-Order          | Focused        | Visible
-----------------------+------------------+----------------+--------
com.bskyb.epgui         | 100              | Yes            | Yes
com.netflix.nrdp        | 50               | No             | No
youtube_youtube_tv      | 25               | No             | No

    Summary: 3 window client(s)
```

**Fields:**
- **App ID**: Application identifier
- **Z-Order**: Window stacking order (higher = on top)
- **Focused**: Currently focused window (Yes/No)
- **Visible**: Visible on screen (Yes/No)

**Focused Application:**
- Shows which app has input focus

---

## Complete AppManager Dump

```
APP MANAGER DIAGNOSTICS
=======================

RUNTIME SECTION:
    Status: ok

LOADED APPLICATIONS:
===================
[... table as shown above ...]

INSTALLED APPLICATIONS:
======================
[... table as shown above ...]


PACKAGE SECTION:
    Status: ok

PACKAGE MANAGER:
===============
[... table as shown above ...]


RESOURCE SECTION:
    Status: ok

RESOURCE MONITOR:
===============
[... table as shown above ...]


WINDOW SECTION:
    Status: ok

WINDOW MANAGER:
===============
[... table as shown above ...]
```

---

## Usage

### Get all AppManager diagnostics:
```bash
entos-diagnostics dumpsys --section runtime --section package --section resource --section window
```

### Get specific section:
```bash
entos-diagnostics dumpsys --section runtime  # Only loaded/installed apps
entos-diagnostics dumpsys --section package  # Only packages
entos-diagnostics dumpsys --section resource # Only resource stats
entos-diagnostics dumpsys --section window   # Only window state
```

### Get focused AppManager diagnostics:
```bash
entos-diagnostics appmanager              # Complete AppManager dump
entos-diagnostics appmanager --loaded     # Loaded apps only
entos-diagnostics appmanager --installed  # Installed apps only
entos-diagnostics appmanager --packages   # Packages only
entos-diagnostics appmanager --resources  # Resource stats only
entos-diagnostics appmanager --window     # Window state only
```

---

## Implementation Files

### Header Files
```
include/entos/AppManagerFormatter.h
    - AppManagerFormatter class
    - Format functions for each data type
    - Table generation utilities

include/entos/AppManagerDumpsys.h
    - AppManagerDumpsys class
    - Query and dump methods
    - Integration with main dumpsys
```

### Implementation Files
```
src/application/AppManagerFormatter.cpp
    - Table formatting logic
    - Column alignment
    - Summary calculations

src/application/AppManagerDumpsys.cpp (to be created)
    - RPC query logic
    - Dumpsys integration
    - Output generation
```

### Configuration
```
config/providers.json
    - 5 AppManager providers with field mappings
    - JSON-RPC method definitions
    - Verification status
```

---

## Data Flow

```
entos-diagnostics dump/app <options>
          ↓
    AppManagerDumpsys::query()
          ↓
    RpcClient.call() → Thunder plugins
          ↓
    Parse JSON responses
          ↓
    AppManagerFormatter::format*()
          ↓
    Human-readable table output
          ↓
    Display to terminal/file
```

---

## Output Characteristics

### Table Format
- **Columns**: Aligned with fixed widths
- **Separator**: Dash-dash-dash lines between header and rows
- **Alignment**: Left-aligned text, right-aligned numbers
- **Missing Data**: `--` indicates unavailable fields

### Summary Lines
- Below each table showing counts
- Example: "Summary: 2 app(s) loaded"

### Sections
- Clear headers with underlines
- Status indicators (ok/error/partial)
- Hierarchical organization

### Performance
- No JSON parsing overhead
- Direct string formatting
- Minimal memory usage
- Fast terminal rendering

---

## Example: Complete Output

```
APP MANAGER DIAGNOSTICS
=======================

RUNTIME SECTION:
    Status: ok

LOADED APPLICATIONS:
===================

App ID                  | Instance ID         | State            | PID
-----------------------+-----------------------+------------------+-------
com.netflix.nrdp        | instance-001        | running          | 1234
com.bskyb.epgui         | instance-002        | running          | 5678

    Summary: 2 app(s) loaded

INSTALLED APPLICATIONS:
======================

App ID                  | Name                | Version          | Category
-----------------------+---------------------+------------------+----------------
com.netflix.nrdp        | Netflix             | 100.140.1.22110  | entertainment
com.bskyb.epgui         | Sky EPG UI          | 4.22.2           | system

    Summary: 2 app(s) installed


PACKAGE SECTION:
    Status: ok

PACKAGE MANAGER:
===============

    Cache Status:
        Used: 504 MB
        Limit: 100 MB

    Packages:

Package Name            | Version          | Size         | Type    | Installed
------------------------+------------------+--------------+---------+----------
com.netflix.nrdp        | 100.140.1.22110  | 52 MB        | zip     | Yes
com.bskyb.epgui         | 4.22.2           | 11 MB        | zip     | Yes

    Summary: 2 package(s) in database


RESOURCE SECTION:
    Status: ok

RESOURCE MONITOR:
===============

    Total Resources:
        CPU: 15.7%
        Memory: 2048 MB

    Per-Application Stats:

App ID                  | CPU (%)          | Memory (MB)    | Timestamp
-----------------------+------------------+----------------+---------------
com.netflix.nrdp        | 5.2              | 128            | 1730977661
com.bskyb.epgui         | 3.1              | 256            | 1730977661

    Summary: 2 app(s) monitored


WINDOW SECTION:
    Status: ok

WINDOW MANAGER:
===============

    Focused Application: com.bskyb.epgui

    Window Clients:

App ID                  | Z-Order          | Focused        | Visible
-----------------------+------------------+----------------+--------
com.bskyb.epgui         | 100              | Yes            | Yes
com.netflix.nrdp        | 50               | No             | No

    Summary: 2 window client(s)
```

---

## Configuration Integration

All providers are configured in `config/providers.json`:

```json
{
  "id": "app.loaded",
  "section": "runtime",
  "callsigns": ["org.rdk.AppManager"],
  "operation": "getLoadedApps",
  "fieldMappings": {
    "app.loaded.count": {"path": ["loaded_apps"], "embeddedJson": false},
    "app.loaded.summary": {"path": ["loaded_apps"], "embeddedJson": true}
  },
  "enabled": false,
  "verification": "provisional"
}
```

**Note**: Providers are disabled by default (enabled=false) pending device verification.

---

## Migration from JSON to Tables

### Before (JSON output):
```json
{
  "app.loaded.count": 2,
  "app.loaded.summary": "[{\"appId\": \"...\", ...}]"
}
```

### After (Table format):
```
LOADED APPLICATIONS:
===================

App ID                  | Instance ID         | State
-----------------------+-----------------------+------------------
com.netflix.nrdp        | instance-001        | running

    Summary: 1 app(s) loaded
```

---

## Next Steps

1. ✅ Define human-readable output format (this document)
2. ✅ Create AppManagerFormatter class
3. ✅ Create AppManagerDumpsys handler
4. 🔄 Integrate with main.cpp for new command
5. 🔄 Add CLI argument parsing for --appmanager
6. 🔄 Test with mock data
7. 🔄 Device verification and enablement

---

**Created**: 2026-11-08  
**Format**: Human-readable tables (not JSON)  
**Status**: Ready for implementation  
**Integration**: entos-diagnostics main dumpsys system
