# Deployment Checklist

**Date:** November 9, 2024  
**Build:** `2e1abd5`  
**Binary Size:** 686 KB  
**Build Status:** ✅ 0 errors, 0 warnings

---

## Pre-Deployment Verification ✅

- [x] Binary built successfully
  - Size: 686 KB
  - Type: Mach-O 64-bit executable arm64
  - Location: `build/entos-diagnostics`

- [x] Configuration file ready
  - Location: `config/providers.json`
  - Updates: Embedded JSON support, 9 package fields

- [x] Code changes committed
  - Commit: `2e1abd5` - Handle Thunder's embedded JSON wrapping
  - Previous: `3d767af` - Add missing fields to package section contract
  - Previous: `fc137c8` - Implement dumpsys-compatible output

- [x] Documentation complete
  - DEPLOYMENT_SUMMARY.md ✅
  - QUICK_REFERENCE.md ✅
  - This checklist ✅

---

## Deployment Steps

### Step 1: Backup Current Files (Optional but Recommended)
```bash
ssh root@xione-bcm-flex2 << 'EOF'
# Backup existing binary
cp /usr/bin/entos-diagnostics /usr/bin/entos-diagnostics.backup.$(date +%s)

# Backup existing config
cp /etc/entos-diagnostics/providers.json /etc/entos-diagnostics/providers.json.backup.$(date +%s)

echo "Backups created"
EOF
```
- [ ] Backups created on device

### Step 2: Deploy New Binary
```bash
scp /Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics/build/entos-diagnostics \
    root@xione-bcm-flex2:/usr/bin/entos-diagnostics
```
- [ ] Binary copied to device
- [ ] Verify: `ssh root@xione-bcm-flex2 "ls -lh /usr/bin/entos-diagnostics"`

### Step 3: Deploy New Configuration
```bash
scp /Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics/config/providers.json \
    root@xione-bcm-flex2:/etc/entos-diagnostics/providers.json
```
- [ ] Config copied to device
- [ ] Verify: `ssh root@xione-bcm-flex2 "cat /etc/entos-diagnostics/providers.json | head -20"`

---

## Post-Deployment Verification

### Test 1: Binary Accessibility
```bash
ssh root@xione-bcm-flex2 "entos-diagnostics --help"
```

Expected:
```
Usage: entos-diagnostics [dumpsys|plugins|list] [options]
  ...
Exit: 0 complete, 1 partial diagnostic result, 2 invalid input/configuration.
```

- [ ] Command returns help text without errors

### Test 2: List Available Sections
```bash
ssh root@xione-bcm-flex2 "entos-diagnostics list"
```

Expected: Should list sections including:
- package  PACKAGE MANAGER
- runtime  RUNTIME MANAGER
- window   WINDOW MANAGER
- resource RESOURCE MONITOR

- [ ] All sections listed

### Test 3: Package Manager (Main Fix) - PRIMARY TEST
```bash
ssh root@xione-bcm-flex2 "entos-diagnostics dumpsys --section package --format text"
```

**Before Fix:**
```
ERROR: package.list: result violates configured response contract
```

**After Fix - Expected Output:**
```
PACKAGE MANAGER:
    Installer:
        Download Threads: [number]
        Requests in Progress: [list or "None"]
        Requests Pending: [list or "None"]
    
    Package Database:
        Compatible Platform Ids: [IDs]
        Storage Dir: [path]
        ...
        
    Package Cache:
        Cache Size: [size]
        Cache Limit: [limit]
        Entries:
            [table with columns: App, Version, Size, Last Used, Format, Path]
```

- [ ] No error messages
- [ ] Displays formatted table
- [ ] Contains expected sections

### Test 4: Package Manager JSON (Verify Field Extraction)
```bash
ssh root@xione-bcm-flex2 "entos-diagnostics dumpsys --section package --format json | head -100"
```

Expected: JSON with "data" section containing:
```json
"data": {
  "platformIds": "...",
  "storageDir": "...",
  "packages": [...],
  "cacheSize": ...,
  "cacheLimit": ...,
  "cacheEntries": [...],
  "downloadThreads": ...,
  "inProgress": [...],
  "pending": [...]
}
```

- [ ] All 9 fields present in JSON output
- [ ] No extraction errors

### Test 5: Verify No Regressions (Optional Sections)
```bash
ssh root@xione-bcm-flex2 "entos-diagnostics list" | awk '{print $1}' | while read section; do
  echo "Testing section: $section"
  ssh root@xione-bcm-flex2 "entos-diagnostics dumpsys --section $section --format text 2>&1" | head -5
done
```

- [ ] No unexpected errors in other sections
- [ ] Each section either displays data or clear error message

---

## Troubleshooting

### Issue: "ERROR: Invalid field mapping contract"
**Diagnosis:** Section contract still incomplete

**Solution:**
```bash
# Verify config was deployed
ssh root@xione-bcm-flex2 "grep -c 'platformIds' /etc/entos-diagnostics/providers.json"
# Should output: 1

# If 0, config not deployed. Re-run step 3.
```

### Issue: "ERROR: result violates configured response contract"
**Diagnosis:** Embedded JSON parsing not working

**Solution:**
```bash
# Verify flag is set
ssh root@xione-bcm-flex2 "grep 'resultIsEmbeddedJson' /etc/entos-diagnostics/providers.json"
# Should show: "resultIsEmbeddedJson": true

# Verify binary has been updated
ssh root@xione-bcm-flex2 "ls -l /usr/bin/entos-diagnostics"
# Check timestamp matches deployment time

# Re-deploy binary if needed
scp build/entos-diagnostics root@xione-bcm-flex2:/usr/bin/entos-diagnostics
```

### Issue: "ERROR: in Json::Value::find(key, end, found): requires objectValue or nullValue"
**Diagnosis:** Window formatter receiving wrong data type

**Solution:**
```bash
# Test with JSON format to see actual data
ssh root@xione-bcm-flex2 "entos-diagnostics dumpsys --section window --format json 2>&1 | head -50"

# Check provider config for window
ssh root@xione-bcm-flex2 "grep -A 10 'window.clients' /etc/entos-diagnostics/providers.json"
```

### Issue: "ERROR: Unverified provider cannot be enabled"
**Diagnosis:** Provider verification still set to false

**Solution:**
```bash
# Check provider config
ssh root@xione-bcm-flex2 "grep 'verification' /etc/entos-diagnostics/providers.json"
# Should show: "verification": "verified" (not "provisional")

# If still "provisional", update config and re-deploy
```

---

## Rollback Plan

If deployment fails, revert to previous version:

```bash
# Option 1: Restore from backup
ssh root@xione-bcm-flex2 << 'EOF'
cp /usr/bin/entos-diagnostics.backup.* /usr/bin/entos-diagnostics
cp /etc/entos-diagnostics/providers.json.backup.* /etc/entos-diagnostics/providers.json
echo "Rollback complete"
EOF

# Option 2: Rebuild from source (if on device)
ssh root@xione-bcm-flex2 << 'EOF'
cd /opt/entos-diagnostics && cmake --build build --target entos-diagnostics
EOF
```

- [ ] Rollback procedure documented and tested (optional)

---

## Success Criteria

All of the following must be true:

1. ✅ Binary deployed and accessible
2. ✅ Configuration deployed with embedded JSON support
3. ✅ `entos-diagnostics --help` works
4. ✅ `entos-diagnostics list` shows all sections
5. ✅ **`entos-diagnostics dumpsys --section package --format text`** displays formatted table
6. ✅ No "result violates configured response contract" error
7. ✅ JSON output shows all 9 package fields
8. ✅ Other sections still work (no regressions)

---

## Sign-Off

- **Deployment Date:** _____________
- **Deployed By:** _____________
- **Device:** xione-bcm-flex2
- **All Tests Passed:** [ ] Yes  [ ] No
- **Notes:**
  ```
  
  
  ```

---

## Technical Reference

### What Changed
1. Added `resultIsEmbeddedJson` field to Provider struct
2. AppCollector now parses JSON strings before type validation
3. Section contract now includes all 9 package manager fields
4. DumpsysFormatter.formatPackageManager() completely rewritten for table output
5. Data access layer fixed: formatters receive section["data"] instead of full section

### Why These Changes
- Thunder wraps JSON responses as strings in the result field
- Old code failed type validation before parsing the string
- New code parses the string first, then validates type
- This enables proper field extraction from nested Thunder API responses

### Commits Included
```
2e1abd5 Fix: Handle Thunder's embedded JSON result wrapping
3d767af Fix: Add missing fields to package section contract
fc137c8 Implement dumpsys-compatible output for PackageManager
febf674 Fix: Pass section data object to formatters, not full section
3ac7897 Rephase the parser code to get readable format
```

---

**Document Location:** `/Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics/DEPLOYMENT_CHECKLIST.md`  
**Related Docs:** DEPLOYMENT_SUMMARY.md, QUICK_REFERENCE.md  
**Status:** Ready for deployment
