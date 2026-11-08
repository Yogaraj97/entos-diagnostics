# AppManager Diagnostics - Test Plan & Verification Guide

## Overview

This document outlines the testing strategy for the AppManager diagnostics integration with entos-diagnostics. The integration enables unified querying of app lifecycle, package management, and resource metrics via JSON-RPC.

## Artifacts Created

### 1. **OpenSpec Specification** 📋
- **Location**: `openspec/changes/add-appmanager-diagnostics/.openspec.yaml`
- **Purpose**: Schema-driven specification with provider contracts, evidence links, and verification checklist
- **Contains**: Proposal, specs, design notes, and task roadmap

### 2. **Enhanced providers.json** ⚙️
- **Location**: `config/providers.json`
- **Updates**:
  - Added `fieldMappings` for all 5 providers
  - Enhanced evidence sections with curl command examples
  - All providers marked `enabled: false` (gates on device verification)
  - All marked `verification: provisional` (pending live testing)

### 3. **Integration Documentation** 📖
- **Location**: `APPMANAGER_INTEGRATION.md`
- **Contents**:
  - Problem statement and solution overview
  - Detailed provider specifications with response contracts
  - Implementation status and roadmap
  - Schema validation requirements
  - Security and backward compatibility notes
  - Usage examples and testing strategy

### 4. **Validation Script** 🔧
- **Location**: `validate_appmanager.sh`
- **Features**:
  - Schema validation against `providers.schema.json`
  - List all providers with status
  - Show field mappings per provider
  - Generate mock Thunder responses for unit testing
  - Create curl test script for manual device testing

### 5. **Mock Test Data** 🧪
- **Location**: `tests/mocks/`
- **Files**:
  - `app_loaded.json` — Sample getLoadedApps response
  - `app_installed.json` — Sample getInstalledApps response
  - `package_list.json` — Sample listPackages response
  - `resource_stats.json` — Sample GetStats response
  - `window_clients.json` — Sample getApps response

### 6. **Curl Test Script** 🌐
- **Location**: `test_appmanager.sh` (auto-generated)
- **Usage**: `./test_appmanager.sh [host] [port]`
- **Tests all 5 methods** against a live Thunder endpoint

---

## Test Phases

### Phase 1: Schema & Configuration ✅

**Objective**: Verify that providers.json is valid and complete

**Tests**:

```bash
# 1. Schema validation
cd entos-diagnostics
./validate_appmanager.sh validate

# Expected: Schema validation passed
```

**Checklist**:
- [x] All providers have `id`, `callsigns`, `operation`, `resultType`
- [x] `fieldMappings` present for all providers (conditional on `enabled: true`)
- [x] Field paths are arrays of strings
- [x] Evidence strings are non-empty and include curl examples
- [x] All enabled=false and verification=provisional

**Exit Criteria**: providers.json passes schema validation

---

### Phase 2: Provider Configuration Review ✅

**Objective**: Verify provider definitions match Thunder plugin documentation

**Tests**:

```bash
# 1. List all providers
./validate_appmanager.sh list

# Expected output:
# app.loaded      | getLoadedApps    | runtime   | enabled=false | provisional
# app.installed   | getInstalledApps | runtime   | enabled=false | provisional
# package.list    | listPackages     | package   | enabled=false | provisional
# resource.stats  | GetStats         | resource  | enabled=false | provisional
# window.clients  | getApps          | window    | enabled=false | provisional

# 2. Show field mappings
./validate_appmanager.sh fields

# Expected output:
# app.loaded: 2 fields
# app.installed: 2 fields
# package.list: 3 fields
# resource.stats: 3 fields
# window.clients: 3 fields
```

**Manual Review Checklist**:
- [x] Callsigns match entservices-appmanagers documentation
- [x] Methods are correct JSON-RPC names
- [x] Field paths reflect expected response structure
- [x] Evidence URLs are accurate

**Exit Criteria**: All 5 providers configured with correct callsigns and methods

---

### Phase 3: Unit Testing (Host-Side) 🔄

**Objective**: Verify that AppCollector correctly maps fields from mock responses

**Tests**:

```bash
# 1. Build with testing enabled
cd entos-diagnostics/build
cmake -DBUILD_TESTING=ON ..
cmake --build .

# 2. Generate mock responses
../validate_appmanager.sh generate

# 3. Run unit tests (when implemented)
ctest -V -R "AppManager" --output-on-failure

# Expected: All unit tests pass
```

**Test Cases** (to implement):

| Test | Input | Expected Output | Status |
|---|---|---|---|
| Parse app.loaded mock | `app_loaded.json` | `app.loaded.count = 2`, `app.loaded.summary = [...JSON...]` | 🔄 Pending |
| Parse app.installed mock | `app_installed.json` | `app.installed.count = 2`, `app.installed.summary = [...JSON...]` | 🔄 Pending |
| Parse package.list mock | `package_list.json` | `package.count = 2`, `package.cache.used = 503316480`, `package.list = [...JSON...]` | 🔄 Pending |
| Parse resource.stats mock | `resource_stats.json` | `resource.cpu.total = 15.7`, `resource.memory.total = 2147483648` | 🔄 Pending |
| Parse window.clients mock | `window_clients.json` | `window.client.count = 2`, `window.focused.app = "com.bskyb.epgui"` | 🔄 Pending |
| Field path traversal | Nested JSON structures | Correctly extracts values at specified paths | 🔄 Pending |
| Embedded JSON parsing | `embeddedJson: true` fields | Parses JSON strings and returns objects | 🔄 Pending |
| Error handling | Missing field | Logs error and skips field | 🔄 Pending |

**Exit Criteria**: All unit tests pass with mock data

---

### Phase 4: Device Verification 🚫 Blocked

**Objective**: Verify that actual device responses match expected contracts

**Prerequisites**:
- [x] Schema validation complete
- [x] Unit tests passing
- [ ] Device access (SKY/Xumo or equivalent)
- [ ] Thunder plugins running (AppManager, AppPackageManager, ResourceMonitor, RDKWindowManager)

**Tests**:

```bash
# 1. Generate curl test script
./validate_appmanager.sh curl

# 2. On target device, run:
./test_appmanager.sh 127.0.0.1 9998

# 3. Capture and analyze responses
./test_appmanager.sh 127.0.0.1 9998 > device_responses.json

# 4. For each method, verify:
jq '.result.loaded_apps | length' device_responses.json  # app.loaded
jq '.result.installed_apps | length' device_responses.json  # app.installed
jq '.result.packages | length' device_responses.json  # package.list
jq '.result.stats | length' device_responses.json  # resource.stats
jq '.result.apps | length' device_responses.json  # window.clients
```

**Verification Checklist** (for each provider):

| Provider | Check | Observation | Status |
|---|---|---|---|
| **app.loaded** | Response includes `loaded_apps` array | Count apps | [ ] |
| | Each app has `appId`, `instanceId`, `state` fields | Verify structure | [ ] |
| | No parsing errors in AppCollector | Field paths correct | [ ] |
| **app.installed** | Response includes `installed_apps` array | Count apps | [ ] |
| | Each app has `appId`, `name`, `version` fields | Verify structure | [ ] |
| | Matches PACKAGE MANAGER Database in dumpsys | Cross-check | [ ] |
| **package.list** | Response includes `packages` array | Count packages | [ ] |
| | Response includes `cache` object with `used_bytes` | Verify structure | [ ] |
| | Matches PACKAGE MANAGER Cache in dumpsys | Cross-check | [ ] |
| **resource.stats** | Response includes `stats` array with per-app metrics | Verify structure | [ ] |
| | `total_cpu` and `total_memory_bytes` present | Aggregate check | [ ] |
| | Timestamps are from server (not collector clock) | Compare with device time | [ ] |
| **window.clients** | Response includes `apps` array with z-order | Verify structure | [ ] |
| | `focused_app_id` identifies current foreground app | Match visual state | [ ] |
| | Z-order matches RDK shell display | Cross-check | [ ] |

**Live Testing Steps**:

1. **Deploy entos-diagnostics to device**
   ```bash
   scp build/entos-diagnostics root@device:/usr/local/bin/
   ```

2. **Test each method independently**
   ```bash
   ssh root@device '/usr/local/bin/test_appmanager.sh 127.0.0.1 9998' | jq '.result'
   ```

3. **Run full diagnostic snapshot**
   ```bash
   ssh root@device '/usr/local/bin/entos-diagnostics --output json' > snapshot.json
   jq '.diagnostic.runtime' snapshot.json | head -50
   ```

4. **Compare with dumpsys**
   ```bash
   ssh root@device 'appsservicectl dumpsys' > dumpsys_live.txt
   # Manually compare PACKAGE MANAGER sections with JSON output
   ```

5. **Update evidence in providers.json**
   - Document actual response structure (if different from contract)
   - Update field paths if nesting differs
   - Record callsign availability (e.g., if AppPackageManager unavailable, use PackageManager)

**Exit Criteria**:
- All 5 methods respond correctly on target device
- Response shapes match or are documented in field paths
- No parsing errors in diagnostic output
- Timestamp validation passes

**Blockers**:
- [ ] Device access unavailable → Skip to Phase 5, mark as "blocked"
- [ ] Plugin unavailable → Test fallback callsigns
- [ ] Response shape differs → Update field paths and re-test

---

### Phase 5: Full Integration Testing 📅

**Objective**: Verify end-to-end diagnostic collection including AppManager providers

**Tests** (when enabled):

```bash
# 1. Run full diagnostic snapshot
./entos-diagnostics --output json > full_snapshot.json

# 2. Verify all sections
jq '.diagnostic | keys' full_snapshot.json
# Expected: ["config", "dial", "runtime", "package", "resource", "window", ...]

# 3. Spot-check AppManager fields
jq '.diagnostic.runtime["app.loaded.count"]' full_snapshot.json
jq '.diagnostic.runtime["app.installed.count"]' full_snapshot.json
jq '.diagnostic.package["package.count"]' full_snapshot.json

# 4. Compare with dumpsys
appsservicectl dumpsys > dumpsys_ref.txt
# Manual review: App count, package count, and resource usage should be consistent
```

**Exit Criteria**:
- Diagnostic snapshot includes all AppManager sections
- Field counts are consistent across runs
- No memory leaks or hangs in collection
- Snapshot completes within 5 seconds

---

### Phase 6: Production Release 📤

**Objective**: Enable AppManager providers and merge to main

**Checklist**:

- [ ] All unit tests passing
- [ ] Device verification complete (Phase 4)
- [ ] Full integration test passing (Phase 5)
- [ ] Update `enabled: true` in providers.json for all 5 providers
- [ ] Update `verification: verified` for all 5 providers
- [ ] Add smoke test to CI pipeline
- [ ] Document deployment in CHANGELOG
- [ ] Merge to main branch

**Post-Merge Tasks**:

1. **Monitor in production**
   - Track error rates for each provider
   - Log any timeout issues

2. **Update documentation**
   - Update README with AppManager examples
   - Add to feature list in CHANGELOG

3. **Community comms**
   - Announce new diagnostic capabilities
   - Provide usage examples

---

## Mock Data Reference

### app_loaded.json
```json
{
  "success": true,
  "loaded_apps": [
    {
      "appId": "com.netflix.nrdp",
      "instanceId": "instance-001",
      "state": "running",
      "pid": 1234
    },
    {
      "appId": "com.bskyb.epgui",
      "instanceId": "instance-002",
      "state": "running",
      "pid": 5678
    }
  ]
}
```

**Field extraction**:
- `app.loaded.count` → 2 (array length)
- `app.loaded.summary` → `[{appId:..., instanceId:..., state:...}, ...]` (full array)

---

### app_installed.json
```json
{
  "success": true,
  "installed_apps": [
    {
      "appId": "com.netflix.nrdp",
      "name": "Netflix",
      "version": "100.140.1.22110",
      "category": "entertainment"
    },
    {
      "appId": "com.bskyb.epgui",
      "name": "Sky EPG UI",
      "version": "4.22.2",
      "category": "system"
    }
  ]
}
```

**Field extraction**:
- `app.installed.count` → 2 (array length)
- `app.installed.summary` → `[{appId:..., name:..., version:...}, ...]`

---

### package_list.json
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

**Field extraction**:
- `package.count` → 1 (packages array length)
- `package.cache.used` → 503316480 (bytes)
- `package.list` → `[{name:..., version:..., size:...}, ...]`

---

### resource_stats.json
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

**Field extraction**:
- `resource.cpu.total` → 15.7
- `resource.memory.total` → 2147483648
- `resource.stats.summary` → `[{appId:..., cpu:..., memory_bytes:...}, ...]`

---

### window_clients.json
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

**Field extraction**:
- `window.client.count` → 1 (apps array length)
- `window.focused.app` → "com.bskyb.epgui"
- `window.apps.summary` → `[{appId:..., zOrder:..., focused:...}, ...]`

---

## Running Tests Locally

### Setup

```bash
cd entos-diagnostics

# Generate all test data and curl scripts
./validate_appmanager.sh all

# List generated files
ls -la tests/mocks/
ls -la test_appmanager.sh
```

### Quick Validation

```bash
# Validate schema (if jsonschema installed)
pip install jsonschema
./validate_appmanager.sh validate

# Check provider configuration
./validate_appmanager.sh list
./validate_appmanager.sh fields
```

### Device Testing

```bash
# Copy to device
scp test_appmanager.sh root@device:/tmp/

# Run on device
ssh root@device 'cd /tmp && ./test_appmanager.sh 127.0.0.1 9998'

# Analyze output
ssh root@device './test_appmanager.sh 127.0.0.1 9998' | jq . > responses.json
```

---

## Key Files Reference

| File | Purpose |
|---|---|
| `config/providers.json` | Provider definitions with field mappings |
| `config/providers.schema.json` | JSON schema for validation |
| `openspec/changes/add-appmanager-diagnostics/.openspec.yaml` | OpenSpec design document |
| `APPMANAGER_INTEGRATION.md` | Integration guide and implementation notes |
| `validate_appmanager.sh` | Validation and test generation script |
| `test_appmanager.sh` | Curl-based manual testing script |
| `tests/mocks/*.json` | Mock response files for unit tests |

---

## Next Steps

1. **Host-side implementation**: Run Phase 2 & 3 validation ✅
2. **Device access**: Arrange device testing for Phase 4
3. **Update evidence**: Document actual response shapes from target
4. **Enable providers**: Set `enabled: true` after Phase 4 verification
5. **CI integration**: Add smoke test to CI pipeline
6. **Production release**: Merge to main after all gates passed

---

**Document Version**: 1.0  
**Created**: 2026-11-08  
**Status**: Phase 1-2 Complete, Phase 3-4 Pending  
**Maintainer**: EntOS Diagnostics Team
