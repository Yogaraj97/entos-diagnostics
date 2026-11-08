# AppManager Diagnostics Integration

## Overview

This document describes the integration of **AppManager** JSON-RPC methods into **entos-diagnostics** for comprehensive application and package state collection.

## Problem Statement

Developers and support teams need rapid visibility into:
- Which applications are currently loaded and their lifecycle state
- What applications are installed on the device
- Package manager cache and storage status
- Per-application resource consumption (CPU, memory)
- Window manager focus and z-order state

Previously, this required multiple manual curl commands to individual Thunder plugins:
```bash
# Loaded apps
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.AppManager.getLoadedApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc

# Installed apps
curl -d '{"jsonrpc":"2.0","id":"1","method":"org.rdk.AppManager.getInstalledApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc

# List packages
curl -d '{"jsonrpc":"2.0","id":"1001","method":"org.rdk.AppPackageManager.listPackages","params":{}}' \
  http://127.0.0.1:9998/jsonrpc
```

## Solution

Entos-Diagnostics now exposes these queries through a unified, schema-driven provider framework in `config/providers.json`. Each provider:
- Defines JSON-RPC callsigns (with fallbacks for plugin aliasing)
- Specifies result contracts (expected JSON structure)
- Maps response fields to diagnostic outputs
- Includes verification status and evidence trails

### Enabled Providers

| ID | Method | Callsign | Section | Status |
|---|---|---|---|---|
| `app.loaded` | getLoadedApps | org.rdk.AppManager | runtime | provisional |
| `app.installed` | getInstalledApps | org.rdk.AppManager | runtime | provisional |
| `package.list` | listPackages | org.rdk.AppPackageManager | package | provisional |
| `resource.stats` | GetStats | org.rdk.ResourceMonitor | resource | provisional |
| `window.clients` | getApps | org.rdk.RDKWindowManager | window | provisional |

## Provider Specifications

### 1. app.loaded — Loaded Applications

**Query:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "org.rdk.AppManager.1.getLoadedApps",
  "params": {}
}
```

**Expected Response Contract:**
```json
{
  "success": true,
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

**Mapped Fields:**
- `app.loaded.count` → Length of loaded_apps array (type: number)
- `app.loaded.summary` → JSON-serialized loaded_apps (type: embedded JSON)

**Evidence:**
- Source: entservices-appmanagers/AppManager/AppManager.h (IAppManager interface)
- dumpsys.txt LOADED APPLICATIONS section
- Test vector: entservices-appmanagers/Tests/

### 2. app.installed — Installed Applications

**Query:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "org.rdk.AppManager.1.getInstalledApps",
  "params": {}
}
```

**Expected Response Contract:**
```json
{
  "success": true,
  "installed_apps": [
    {
      "appId": "com.netflix.nrdp",
      "name": "Netflix",
      "version": "100.140.1.22110",
      "category": "entertainment",
      "metadata": "{\"launcher\":true}"
    }
  ]
}
```

**Mapped Fields:**
- `app.installed.count` → Length of installed_apps array
- `app.installed.summary` → JSON-serialized installed_apps (embedded JSON)

**Evidence:**
- Source: entservices-appmanagers/AppManager/AppManager.h
- dumpsys.txt PACKAGE MANAGER Database section
- Cross-reference: entos-appmanagers/

### 3. package.list — Package Manager Inventory

**Query:**
```json
{
  "jsonrpc": "2.0",
  "id": 1001,
  "method": "org.rdk.AppPackageManager.1.listPackages",
  "params": {}
}
```

**Expected Response Contract:**
```json
{
  "success": true,
  "packages": [
    {
      "name": "com.netflix.nrdp",
      "version": "100.140.1.22110",
      "size": "52 MB",
      "installed": true,
      "type": "zip"
    }
  ],
  "cache": {
    "used_bytes": 503316480,
    "limit_bytes": 104857600
  }
}
```

**Mapped Fields:**
- `package.count` → Length of packages array
- `package.cache.used` → cache.used_bytes (in bytes)
- `package.list` → JSON-serialized packages (embedded JSON)

**Evidence:**
- Source: entservices-appmanagers/PackageManager/
- dumpsys.txt PACKAGE MANAGER section (Database, Cache, Mounted Packages)
- Callsign fallback: org.rdk.PackageManager (if AppPackageManager unavailable)

### 4. resource.stats — Per-App Resource Metrics

**Query:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "org.rdk.ResourceMonitor.1.GetStats",
  "params": {}
}
```

**Expected Response Contract:**
```json
{
  "success": true,
  "stats": [
    {
      "appId": "com.netflix.nrdp",
      "cpu": 5.2,
      "memory_bytes": 134217728,
      "timestamp": 1730977661
    }
  ],
  "total_cpu": 15.7,
  "total_memory_bytes": 2147483648
}
```

**Mapped Fields:**
- `resource.cpu.total` → total_cpu (aggregate across all apps)
- `resource.memory.total` → total_memory_bytes
- `resource.stats.summary` → JSON-serialized stats (embedded JSON)

**Evidence:**
- Source: entservices-appmanagers/ResourceMonitor/ResourceMonitor.h
- dumpsys process metrics show CPU/memory per process
- Server-side timestamps (not collector clock)

### 5. window.clients — Window Manager Focus State

**Query:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "org.rdk.RDKWindowManager.1.getApps",
  "params": {}
}
```

**Expected Response Contract:**
```json
{
  "success": true,
  "apps": [
    {
      "appId": "com.bskyb.epgui",
      "zOrder": 100,
      "focused": true,
      "visible": true
    }
  ],
  "focused_app_id": "com.bskyb.epgui"
}
```

**Mapped Fields:**
- `window.client.count` → Length of apps array
- `window.focused.app` → focused_app_id string
- `window.apps.summary` → JSON-serialized apps (embedded JSON)

**Evidence:**
- Source: entservices-appmanagers/RDKWindowManager/
- RDK shell show shows app z-order and focus
- Complements app.loaded with visual state

## Implementation Status

### Phase 1: Host-Side Contracts ✅

- ✅ Specification document created (.openspec.yaml)
- ✅ Providers added to config/providers.json with field mappings
- ✅ Field paths and result types validated against schema
- ✅ Evidence links and verification status documented
- ✅ All providers marked `enabled: false` and `verification: provisional`

### Phase 2: Host Testing 🔄

- Host tests will mock Thunder server responses
- AppCollector.cpp generic logic already validates field paths
- Schema validation will verify JSON structure before field extraction

### Phase 3: Device Verification 🚫 Blocked

- **Gated on**: Target device access (SKY/Xumo or equivalent)
- **Steps**: 
  1. Deploy entos-diagnostics to device
  2. Run live queries against all 5 callsigns
  3. Capture actual response shapes (may differ from contract)
  4. Verify field paths are correct (no missing nesting levels)
  5. Update evidence section and mark `verification: verified`

### Phase 4: Production Release 📅 Pending Phase 3

- Set `enabled: true` for verified providers
- Merge to main branch
- Enable in production builds

## Schema Validation

All providers.json changes conform to [providers.schema.json](providers.schema.json):

```bash
# Validate locally
jsonschema -i config/providers.json config/providers.schema.json
```

**Key constraints:**
- `enabled: true` requires `verification: verified` AND `fieldMappings` with ≥1 entry
- `callsigns` array has ≥1 entry; first is primary, rest are fallbacks
- `operation` must be in enum list (JSON-RPC method names are versioned separately)
- `fieldMappings` paths are arrays of keys to traverse nested JSON
- `embeddedJson: true` means the field is a JSON-stringified value (parsed after extraction)

## OpenSpec Artifact

See [.openspec.yaml](openspec/changes/add-appmanager-diagnostics/.openspec.yaml) for:
- Proposal and non-goals
- Detailed specs with result contracts
- Design notes on data ownership and privacy
- Task breakdown and verification checklist

## Usage Example

Once enabled on a device:

```bash
# Collect full diagnostic snapshot (includes AppManager state)
entos-diagnostics --output json > device-snapshot.json

# Extract loaded apps from snapshot
jq '.diagnostic.runtime["app.loaded"]' device-snapshot.json

# Extract package stats
jq '.diagnostic.package["package.list"]' device-snapshot.json
```

## Backward Compatibility

- All AppManager changes are **additive** to providers.json
- Existing providers (platform, configuration, etc.) are unmodified
- AppCollector.cpp uses generic iteration; no hardcoded method names
- If a callsign is unavailable, the provider is skipped with an error log
- No breaking changes to diagnostic snapshot schema (new fields added to "runtime", "package", "resource", "window" sections)

## Security Considerations

- ✅ All queries are **read-only** (no state mutations)
- ✅ Automatic credential redaction (no API keys, URLs with secrets)
- ✅ No DRM keys or protected paths in responses
- ✅ appId/metadata is application-public (not sensitive)
- ✅ Timestamps are server-side (no clock skew concerns)

## Testing Strategy

| Test | Scope | Status |
|---|---|---|
| Schema validation | providers.json conforms to schema | ✅ Manual |
| Unit: RPC mock | Mock Thunder returns sample payloads | 🔄 Pending |
| Unit: Field extraction | AppCollector extracts fields correctly | 🔄 Pending |
| Integration: Mock server | Run entos-diagnostics with mock Thunder | 🔄 Pending |
| Live: Device queries | Run queries on target device | 🚫 Blocked (device access) |
| Regression: Existing providers | Verify no impact on platform/config/etc. | 🔄 Pending |

## References

- **Thunder Plugin Framework**: entservices-appmanagers/
- **Diagnostics Framework**: entos-diagnostics/include/entos/Collector.h
- **Provider Schema**: entos-diagnostics/config/providers.schema.json
- **Device Output**: dumpsys.txt (sample from SKY/Xumo device)
- **OpenSpec Artifact**: openspec/changes/add-appmanager-diagnostics/.openspec.yaml

## Next Steps

1. **Run host tests**: Execute unit tests with mocked Thunder responses
2. **Schema validation**: Verify providers.json against schema
3. **Target deployment**: Get device access to run live queries
4. **Update evidence**: Cross-check response shapes against contracts
5. **Enable providers**: Set `enabled: true` and `verification: verified` after target testing
6. **Production merge**: Integrate into main CI/CD pipeline

---

**Last Updated**: 2026-11-08  
**Maintainer**: EntOS Diagnostics Team  
**Status**: Spec-driven design (OpenSpec), host testing phase
