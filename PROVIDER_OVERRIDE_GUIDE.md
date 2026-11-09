# Provider Override Configuration - Usage Guide

## Overview

The `entos-diagnostics` binary now supports provider configuration overrides without modifying the base configuration file.

**Architecture:**
- **Base Config** (Read-Only): `/etc/entos-diagnostics/providers.json`
- **Override Config** (Writable): `/opt/entos-diagnostics/providers.json.override`
- **Logic**: Provider IDs in override file replace base config entries

---

## Quick Start

### 1. Deploy Updated Binary

```bash
# Build
cd entos-diagnostics
cmake --build build -- -j4

# Deploy
scp build/entos-diagnostics root@xione-bcm-flex2:/usr/bin/
```

### 2. Create Override Directory (on device)

```bash
ssh root@xione-bcm-flex2
mkdir -p /opt/entos-diagnostics
```

### 3. Create Override File (on device)

Copy the example file to use as a template:

```bash
# On your machine
scp config/providers.json.override.example root@xione-bcm-flex2:/opt/entos-diagnostics/providers.json.override
```

OR manually create with your provider settings.

### 4. Test

```bash
# On device
entos-diagnostics dumpsys --section runtime
```

---

## How It Works

### Merge Algorithm

```
FOR EACH provider in /etc/providers.json (base):
  IF provider.id exists in /opt/providers.json.override:
    USE override provider
    MARK as used
  ELSE:
    USE base provider

FOR EACH unused override provider:
  ADD to result
```

### Example

**Base Config** (`/etc/entos-diagnostics/providers.json`):
```json
{
  "providers": [
    {"id": "app.loaded", "enabled": false, ...},
    {"id": "app.installed", "enabled": false, ...},
    {"id": "package.list", "enabled": false, ...}
  ]
}
```

**Override Config** (`/opt/entos-diagnostics/providers.json.override`):
```json
{
  "providers": [
    {"id": "app.loaded", "enabled": true, ...},
    {"id": "custom.new", "enabled": true, ...}
  ]
}
```

**Result**:
- `app.loaded` → from override (enabled)
- `app.installed` → from base (disabled)
- `package.list` → from base (disabled)
- `custom.new` → from override (new provider added)

---

## Usage Scenarios

### Scenario 1: Enable Disabled Providers

**Problem**: All providers in `/etc/providers.json` have `"enabled": false`

**Solution**: Create override with all providers enabled

```bash
# Copy example to device
scp config/providers.json.override.example root@device:/opt/entos-diagnostics/providers.json.override

# Edit to ensure all have "enabled": true and "verification": "verified"
```

### Scenario 2: Disable Specific Providers

```json
{
  "schemaVersion": 1,
  "providers": [
    {
      "id": "app.loaded",
      "section": "runtime",
      ...
      "enabled": false
    }
  ]
}
```

### Scenario 3: Change Provider Operation

```json
{
  "schemaVersion": 1,
  "providers": [
    {
      "id": "app.loaded",
      "operation": "getLoadedAppsV2",
      ...
    }
  ]
}
```

### Scenario 4: Add Custom Provider

```json
{
  "schemaVersion": 1,
  "providers": [
    {
      "id": "custom.mydata",
      "section": "runtime",
      "callsigns": ["com.example.MyPlugin"],
      "interfaceVersion": "1",
      "operation": "getData",
      "readOnly": true,
      "params": {},
      "resultType": "object",
      "evidence": "Custom plugin for diagnostics",
      "verification": "verified",
      "enabled": true,
      "fieldMappings": {
        "data": {
          "path": ["result"],
          "embeddedJson": false
        }
      }
    }
  ]
}
```

---

## File Locations

### On Your Machine (Development)

```
entos-diagnostics/
├── config/
│   ├── providers.json                      (base config)
│   └── providers.json.override.example     (example override)
```

### On Device (Deployment)

```
/etc/entos-diagnostics/
└── providers.json                          (base, read-only)

/opt/entos-diagnostics/
└── providers.json.override                 (override, writable - optional)
```

---

## Deployment Checklist

- [ ] Binary built: `build/entos-diagnostics`
- [ ] Binary deployed: `scp build/entos-diagnostics root@device:/usr/bin/`
- [ ] Override directory created: `mkdir -p /opt/entos-diagnostics` (on device)
- [ ] Override file created (optional): `/opt/entos-diagnostics/providers.json.override`
- [ ] Override file permissions: `chmod 644` (readable by entos-diagnostics process)
- [ ] Tested without override: `entos-diagnostics dumpsys --section runtime`
- [ ] Tested with override: `entos-diagnostics dumpsys --section runtime` (if override exists)

---

## Testing

### Test 1: Without Override File

```bash
ssh root@device
rm -f /opt/entos-diagnostics/providers.json.override
entos-diagnostics dumpsys --section runtime
# Should use base config (/etc/providers.json)
```

### Test 2: With Override File (Providers Enabled)

```bash
# Create override with all providers enabled
cat > /opt/entos-diagnostics/providers.json.override << 'JSON'
{
  "schemaVersion": 1,
  "providers": [
    {
      "id": "app.loaded",
      ...
      "enabled": true,
      "verification": "verified"
    },
    {
      "id": "app.installed",
      ...
      "enabled": true,
      "verification": "verified"
    }
  ]
}
JSON

entos-diagnostics dumpsys --section runtime
# Should use override config, display table format
```

### Test 3: Partial Override

```bash
# Override only app.loaded, others use base
cat > /opt/entos-diagnostics/providers.json.override << 'JSON'
{
  "schemaVersion": 1,
  "providers": [
    {
      "id": "app.loaded",
      ...
      "enabled": true,
      "verification": "verified"
    }
  ]
}
JSON

entos-diagnostics dumpsys --section runtime
# Only app.loaded from override, app.installed from base
```

---

## Fallback Behavior

**If override file doesn't exist**: Binary uses base config from `/etc/providers.json`
- No errors
- Graceful fallback
- No manual intervention needed

**If override file has syntax errors**: Error thrown and reported
- Check JSON syntax
- Verify all required fields
- Use example as template

---

## Configuration Format

### Minimal Override Entry

```json
{
  "id": "provider.id",
  "enabled": true,
  "verification": "verified"
}
```

### Full Override Entry

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
  "evidence": "...",
  "verification": "verified",
  "enabled": true,
  "fieldMappings": {
    "apps": {
      "path": ["loaded_apps"],
      "embeddedJson": false
    }
  }
}
```

---

## Validation

All override providers go through the same validation as base providers:

✅ Schema validation
✅ Required field verification
✅ Verification status check (must be "verified" to enable)
✅ Operation allowlist check
✅ Section contract validation
✅ Field mapping validation

---

## No Other Code Changes Needed

- ✅ Single merge function added
- ✅ Single call in main.cpp
- ✅ No other code modifications required
- ✅ Backward compatible (works without override file)
- ✅ Can be deployed independently

---

## Troubleshooting

### Issue: "Unverified provider cannot be enabled"

**Cause**: Override provider has `"verification": "provisional"`

**Fix**: Change to `"verification": "verified"`

```json
"verification": "verified"  // ✓ Correct
"verification": "provisional" // ✗ Error
```

### Issue: Override file not being used

**Check**:
1. File path: `/opt/entos-diagnostics/providers.json.override` (exact path)
2. File permissions: Readable by entos-diagnostics process
3. JSON syntax: Valid JSON format
4. Provider ID: Matches exactly (case-sensitive)

### Issue: Some providers still disabled

**Check**:
1. Provider in override file has `"enabled": true`
2. Provider has `"verification": "verified"`
3. All required fields present
4. No syntax errors in override JSON

---

## Summary

| Aspect | Details |
|--------|---------|
| **Base Config** | `/etc/entos-diagnostics/providers.json` (read-only) |
| **Override Config** | `/opt/entos-diagnostics/providers.json.override` (optional) |
| **Merge Logic** | Override IDs replace base; new IDs added; fallback if not found |
| **Validation** | Same as base providers |
| **Backward Compatible** | Yes (works without override) |
| **Code Changes** | Minimal (1 function, 1 call) |
| **Deployment** | Binary update only |

---

## Example Use Case

**Goal**: Enable all AppManager providers for diagnostics

**Steps**:

1. Deploy updated binary
2. Create `/opt/entos-diagnostics/providers.json.override` with:
   - All providers: `"enabled": true`
   - All providers: `"verification": "verified"`
3. Run: `entos-diagnostics dumpsys --section runtime`
4. See table output with loaded apps

**Result**: Clean, readable diagnostics without modifying `/etc/providers.json`

---

**Status**: ✅ Ready for Deployment
**Build**: ✅ Successful
**Testing**: Ready on device
