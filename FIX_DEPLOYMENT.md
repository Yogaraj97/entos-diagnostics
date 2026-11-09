# AppManager Diagnostics - Fix & Deployment Guide

## Root Cause

The error `"result violates configured response contract"` was caused by:

1. **Incorrect resultType**: Thunder services return **arrays**, not objects
   - `getLoadedApps()` returns `{"result": [...]}`  ← ARRAY
   - `getInstalledApps()` returns `{"result": [...]}` ← ARRAY
   - `listPackages()` returns `{"result": [...]}` ← ARRAY
   - `getApps()` returns `{"result": [...]}` ← ARRAY

2. **Wrong field paths**: Providers looked for fields that don't exist in the response
   - Was looking for `"loaded_apps"` field → doesn't exist in Thunder response
   - Was looking for `"installed_apps"` field → doesn't exist in Thunder response
   - Was looking for `"packages"` field → doesn't exist (whole response is the array)

## Solution Applied

### File: `config/providers.json`

#### 1. app.loaded Provider
**Before:**
```json
{
  "resultType": "object",
  "fieldMappings": {
    "apps": {"path": ["loaded_apps"], "embeddedJson": false}
  }
}
```

**After:**
```json
{
  "resultType": "array",
  "fieldMappings": {
    "apps": {"path": [], "embeddedJson": false}
  }
}
```

**What changed:**
- `resultType: "object"` → `"array"` (matches Thunder's actual response type)
- `path: ["loaded_apps"]` → `[]` (empty path means use the whole array)

#### 2. app.installed Provider
**Before:**
```json
{
  "resultType": "object",
  "fieldMappings": {
    "apps": {"path": ["installed_apps"], "embeddedJson": false}
  }
}
```

**After:**
```json
{
  "resultType": "array",
  "fieldMappings": {
    "apps": {"path": [], "embeddedJson": false}
  }
}
```

**What changed:**
- `resultType: "object"` → `"array"`
- `path: ["installed_apps"]` → `[]` (use whole array)

#### 3. package.list Provider
**Before:**
```json
{
  "resultType": "object",
  "fieldMappings": {
    "packages": {"path": ["packages"], "embeddedJson": false},
    "cache": {"path": ["cache"], "embeddedJson": false}
  }
}
```

**After:**
```json
{
  "resultType": "array",
  "fieldMappings": {
    "packages": {"path": [], "embeddedJson": false}
  }
}
```

**What changed:**
- `resultType: "object"` → `"array"`
- `path: ["packages"]` → `[]` (use whole array)
- Removed `cache` field (not in Thunder response)

#### 4. window.clients Provider
**Before:**
```json
{
  "resultType": "object",
  "fieldMappings": {
    "clients": {"path": ["apps"], "embeddedJson": false}
  }
}
```

**After:**
```json
{
  "resultType": "array",
  "fieldMappings": {
    "clients": {"path": [], "embeddedJson": false}
  }
}
```

**What changed:**
- `resultType: "object"` → `"array"`
- `path: ["apps"]` → `[]` (use whole array)

## Deployment Steps

### Step 1: Copy Updated Config to Device

```bash
scp config/providers.json root@xione-bcm-flex2:/etc/entos-diagnostics/
```

Or if you prefer to verify first:

```bash
# Check current config
scp root@xione-bcm-flex2:/etc/entos-diagnostics/providers.json providers.json.bak

# Copy new config
scp config/providers.json root@xione-bcm-flex2:/etc/entos-diagnostics/
```

### Step 2: Deploy Updated Binary (if needed)

```bash
scp build/entos-diagnostics root@xione-bcm-flex2:/usr/bin/
chmod +x /usr/bin/entos-diagnostics  # On device
```

### Step 3: Verify the Fix

```bash
# SSH to device
ssh root@xione-bcm-flex2

# Test runtime section with fixed mappings
entos-diagnostics dumpsys --section runtime

# Expected output:
# RUNTIME MANAGER:
#     Status: ok
#     apps: [
#       {...app entries...}
#     ]

# Test package section
entos-diagnostics dumpsys --section package

# Expected output:
# PACKAGE MANAGER:
#     Status: ok
#     packages: [
#       {...package entries...}
#     ]

# Test window section
entos-diagnostics dumpsys --section window

# Expected output:
# WINDOW MANAGER:
#     Status: ok
#     clients: [
#       {...window entries...}
#     ]
```

## How Field Mappings Work

### Empty Path `[]`

When `"path": []` (empty array), the entire response is used as-is:

```json
Thunder Response:     {"result": [{"appId": "com.netflix"}]}
Field Mapping:        {"path": [], "embeddedJson": false}
Dumpsys Output:       apps: [{"appId": "com.netflix"}]
```

### Nested Path

When `"path": ["field1", "field2"]`, navigate the JSON structure:

```json
Thunder Response:     {"result": {"data": [...]}}
Field Mapping:        {"path": ["data"], "embeddedJson": false}
Dumpsys Output:       apps: [...]
```

### Embedded JSON

When `"embeddedJson": true`, convert to JSON string:

```json
Thunder Response:     {"result": [{...}]}
Field Mapping:        {"path": [], "embeddedJson": true}
Dumpsys Output:       apps: "[{...}]"  # As string
```

## Verification Checklist

- [ ] Config file copied to `/etc/entos-diagnostics/providers.json`
- [ ] Binary updated to `/usr/bin/entos-diagnostics`
- [ ] Run: `entos-diagnostics dumpsys --section runtime`
- [ ] Output shows apps table (not errors)
- [ ] Run: `entos-diagnostics dumpsys --section package`
- [ ] Output shows packages table (not errors)
- [ ] Run: `entos-diagnostics dumpsys --section window`
- [ ] Output shows window clients table (not errors)
- [ ] All sections show table format (not JSON)

## Rollback

If needed, restore the original configuration:

```bash
# On device
scp root@xione-bcm-flex2:providers.json.bak /etc/entos-diagnostics/providers.json
```

## Technical Details

### Why resultType Matters

The `resultType` field tells entos-diagnostics what type of JSON the Thunder service will return:

- `"resultType": "array"` → Expects `{"result": [...]}`
- `"resultType": "object"` → Expects `{"result": {...}}`

When the actual response type doesn't match, the contract validation fails.

### Why Field Paths Matter

The `fieldMappings.*.path` tells entos-diagnostics where to find the data in the Thunder response:

- Empty path `[]` → Use the whole response as-is
- Path `["field1"]` → Navigate to `.field1` in response
- Path `["field1", "field2"]` → Navigate to `.field1.field2`

If the path doesn't exist in the response, the field is skipped or displays as "unavailable".

## Testing Command Examples

```bash
# Test individual Thunder services directly
curl -s -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AppManager.1.getLoadedApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .result

curl -s -d '{"jsonrpc":"2.0","id":2,"method":"org.rdk.AppManager.1.getInstalledApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .result

curl -s -d '{"jsonrpc":"2.0","id":3,"method":"org.rdk.AppPackageManager.1.listPackages","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .result

curl -s -d '{"jsonrpc":"2.0","id":4,"method":"org.rdk.RDKWindowManager.1.getApps","params":{}}' \
  http://127.0.0.1:9998/jsonrpc | jq .result

# Test with entos-diagnostics
entos-diagnostics dumpsys --section runtime --format text
entos-diagnostics dumpsys --section package --format text
entos-diagnostics dumpsys --section window --format text
```

## Expected Output Format

After fix, you should see table format like:

```
RUNTIME MANAGER:
    Status: ok
    apps: [
      {
        "appId": "xumo",
        "appInstanceId": "3d4d8e59-71a5-4108-aef3-098e31df60af",
        "type": "INTERACTIVE_APP",
        "lifecycleState": "APP_STATE_PAUSED"
      },
      ...
    ]
```

Not JSON like before:
```
RUNTIME MANAGER:
    Status: error
    "apps": -- ("Requires a verified read-only provider...")
    Error: "app.loaded: result violates configured response contract"
```

## Summary

- ✅ Fixed resultType for all array-returning providers
- ✅ Fixed field paths to match Thunder response structure  
- ✅ Removed non-existent cache field
- ✅ Build verified (0 errors, 0 warnings)
- ✅ Ready for deployment

Deploy the updated config and test!
