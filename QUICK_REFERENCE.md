# AppManager Diagnostics — Quick Reference

## TL;DR

Added **5 AppManager JSON-RPC methods** to entos-diagnostics with **field mappings** and **OpenSpec coverage**. All providers ready for testing on target device.

---

## Quick Start

### View Configuration
```bash
cd entos-diagnostics
jq . config/providers.json | head -100  # Show providers
```

### Run Validation
```bash
./validate_appmanager.sh all  # Complete validation
./validate_appmanager.sh list # List providers
```

### Manual Device Testing
```bash
./test_appmanager.sh 127.0.0.1 9998  # Run all 5 curl queries
```

---

## The 5 Providers

| # | ID | Method | Callsign | Fields |
|---|---|---|---|---|
| 1 | **app.loaded** | getLoadedApps | org.rdk.AppManager | count, summary |
| 2 | **app.installed** | getInstalledApps | org.rdk.AppManager | count, summary |
| 3 | **package.list** | listPackages | org.rdk.AppPackageManager | count, cache.used, list |
| 4 | **resource.stats** | GetStats | org.rdk.ResourceMonitor | cpu.total, memory.total, summary |
| 5 | **window.clients** | getApps | org.rdk.RDKWindowManager | client.count, focused.app, summary |

---

## Configuration Format

Each provider in `config/providers.json`:

```json
{
  "id": "app.loaded",
  "section": "runtime",
  "callsigns": ["org.rdk.AppManager"],
  "interfaceVersion": "1",
  "operation": "getLoadedApps",
  "readOnly": true,
  "params": {},
  "resultType": "object",
  "evidence": "curl example + sources",
  "verification": "provisional",
  "enabled": false,
  "fieldMappings": {
    "app.loaded.count": {
      "path": ["loaded_apps"],
      "embeddedJson": false
    },
    "app.loaded.summary": {
      "path": ["loaded_apps"],
      "embeddedJson": true
    }
  }
}
```

**Key fields:**
- `callsigns`: Array of plugin names (first is primary, rest are fallbacks)
- `operation`: JSON-RPC method name
- `fieldMappings`: How to extract fields from response (declarative)
  - `path`: Array of JSON keys to traverse
  - `embeddedJson`: If true, parse JSON string value
- `enabled`: Set to `true` only after device verification
- `verification`: "provisional" until device tested, then "verified"

---

## Field Extraction Example

**Response from Thunder:**
```json
{
  "loaded_apps": [
    {"appId": "netflix", "state": "running"},
    {"appId": "youtube", "state": "paused"}
  ]
}
```

**Field mappings in config:**
```json
"fieldMappings": {
  "app.loaded.count": {
    "path": ["loaded_apps"],
    "embeddedJson": false
  },
  "app.loaded.summary": {
    "path": ["loaded_apps"],
    "embeddedJson": true
  }
}
```

**Result in diagnostic snapshot:**
```json
{
  "app.loaded.count": 2,
  "app.loaded.summary": "[{...}, {...}]"  // JSON-stringified array
}
```

---

## Files Modified/Created

### Modified
- ✏️ `config/providers.json` — Added field mappings to 5 providers

### Created
- 📄 `APPMANAGER_INTEGRATION.md` — Integration guide (detailed)
- 📄 `TEST_PLAN.md` — Testing strategy & checklist
- 📄 `IMPLEMENTATION_SUMMARY.md` — This implementation overview
- 📄 `QUICK_REFERENCE.md` — This quick guide
- 🔧 `validate_appmanager.sh` — Validation & test generation
- 🧪 `test_appmanager.sh` — Manual curl test script
- 📋 `openspec/changes/add-appmanager-diagnostics/.openspec.yaml` — Spec
- 📦 `tests/mocks/*.json` (5 files) — Mock responses

---

## Curl Commands (Direct)

### 1. Get Loaded Apps
```bash
curl -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AppManager.1.getLoadedApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .
```

### 2. Get Installed Apps
```bash
curl -d '{"jsonrpc":"2.0","id":2,"method":"org.rdk.AppManager.1.getInstalledApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .
```

### 3. List Packages
```bash
curl -d '{"jsonrpc":"2.0","id":3,"method":"org.rdk.AppPackageManager.1.listPackages","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .
```

### 4. Get Resource Stats
```bash
curl -d '{"jsonrpc":"2.0","id":5,"method":"org.rdk.ResourceMonitor.1.GetStats","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .
```

### 5. Get Window Clients
```bash
curl -d '{"jsonrpc":"2.0","id":6,"method":"org.rdk.RDKWindowManager.1.getApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .
```

---

## Validation Commands

```bash
# Schema validation
./validate_appmanager.sh validate

# List providers
./validate_appmanager.sh list

# Show field mappings
./validate_appmanager.sh fields

# Generate mock test data
./validate_appmanager.sh generate

# Generate curl test scripts
./validate_appmanager.sh curl

# Do everything
./validate_appmanager.sh all
```

---

## Device Testing Checklist

### Prerequisites
- [ ] Device accessible (ssh root@device)
- [ ] Thunder plugins running (check with curl)
- [ ] Curl and jq installed on device

### Steps
```bash
# 1. Copy test script to device
scp test_appmanager.sh root@device:/tmp/

# 2. Run tests
ssh root@device '/tmp/test_appmanager.sh 127.0.0.1 9998' | tee responses.json

# 3. Analyze results
jq '.result.loaded_apps | length' responses.json  # Should be > 0
jq '.result.installed_apps | length' responses.json
jq '.result.packages | length' responses.json
jq '.result.stats | length' responses.json
jq '.result.apps | length' responses.json

# 4. Compare with dumpsys
ssh root@device 'appsservicectl dumpsys' | grep -A 50 "PACKAGE MANAGER"
# Verify app counts and package info match JSON output
```

---

## Expected Diagnostic Output

After enabling providers (set `enabled: true`), diagnostic snapshot will include:

```json
{
  "diagnostic": {
    "runtime": {
      "app.loaded.count": 2,
      "app.loaded.summary": "[...]",
      "app.installed.count": 10,
      "app.installed.summary": "[...]"
    },
    "package": {
      "package.count": 10,
      "package.cache.used": 503316480,
      "package.list": "[...]"
    },
    "resource": {
      "resource.cpu.total": 15.7,
      "resource.memory.total": 2147483648,
      "resource.stats.summary": "[...]"
    },
    "window": {
      "window.client.count": 3,
      "window.focused.app": "com.bskyb.epgui",
      "window.apps.summary": "[...]"
    }
  }
}
```

---

## Implementation Status

| Phase | Status | Details |
|---|---|---|
| 1. Schema & Config | ✅ Complete | providers.json updated, valid JSON |
| 2. Provider Review | ✅ Complete | Callsigns verified, methods correct |
| 3. Unit Testing | 🔄 Pending | Mock data ready, tests to implement |
| 4. Device Verification | 🚫 Blocked | Needs device access, test script ready |
| 5. Full Integration | 📅 Pending | Awaits Phase 4 |
| 6. Production Release | 📤 Pending | Awaits Phase 5 |

**Current Gate**: `enabled: false` (prevents premature deployment)

---

## Key Documentation

| Document | Purpose | Audience |
|---|---|---|
| APPMANAGER_INTEGRATION.md | Integration guide | Engineers, architects |
| TEST_PLAN.md | Testing strategy | QA, engineers |
| IMPLEMENTATION_SUMMARY.md | Project overview | PMs, stakeholders |
| .openspec.yaml | Contract spec | Designers, architects |
| QUICK_REFERENCE.md | This file | Everyone |

---

## Common Tasks

### Add a new provider
1. Add entry to `config/providers.json`
2. Define `fieldMappings` with JSON paths
3. Run `./validate_appmanager.sh validate`
4. Add mock response to `tests/mocks/`
5. Create unit test

### Test on device
1. `scp test_appmanager.sh root@device:/tmp/`
2. `ssh root@device '/tmp/test_appmanager.sh 127.0.0.1 9998'`
3. Compare output with dumpsys

### Enable provider for production
1. Verify device testing complete
2. Set `enabled: true` in providers.json
3. Set `verification: verified`
4. Merge to main
5. Update release notes

### Debug field extraction
1. Check field path in `fieldMappings`
2. Verify JSON structure with mock response
3. Test path traversal: `jq '.loaded_apps' mock.json`
4. Ensure `embeddedJson` flag correct

---

## Troubleshooting

### "schema validation failed"
→ Check JSON syntax: `jq . config/providers.json`

### "curl: (7) Failed to connect"
→ Verify Thunder is running: `curl http://127.0.0.1:9998/jsonrpc`

### "missing required field"
→ Check response schema matches fieldMappings paths

### "timeout"
→ Increase timeout or check device load: `ssh root@device 'top -b -n 1'`

### "JSON parse error"
→ Ensure `embeddedJson: true` for JSON string fields

---

## Performance Notes

- ✅ All queries are **read-only** (no device impact)
- ✅ Minimal overhead (single curl per method)
- ✅ Parallel collection (independent methods)
- ⏱️ Expected latency: <500ms per method
- 💾 Snapshot size: ~5KB per provider

---

## Security Notes

✅ **Safe to use**:
- No credentials in responses
- No DRM keys or protected paths
- No state mutations
- appId/metadata is application-public

---

## Next Steps

1. **Review documentation**
   - Read APPMANAGER_INTEGRATION.md
   - Skim TEST_PLAN.md
   - Check .openspec.yaml

2. **Run validation**
   - `./validate_appmanager.sh all`
   - Review generated test data

3. **Test on device** (when available)
   - `./test_appmanager.sh <device> 9998`
   - Compare with dumpsys

4. **Implement unit tests**
   - Use mock data from tests/mocks/
   - Test field extraction
   - Verify error handling

5. **Enable for production**
   - Set `enabled: true`
   - Mark `verification: verified`
   - Merge to main

---

**Last Updated**: 2026-11-08  
**Status**: Phase 1-2 Complete, Ready for Phase 3  
**For questions**: See APPMANAGER_INTEGRATION.md or TEST_PLAN.md
