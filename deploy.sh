#!/bin/bash
# Entos-Diagnostics Deployment Script
# Quick deployment to device with verification

set -e

DEVICE_IP="127.0.0.1"  # Change to actual device IP/hostname
DEVICE_USER="root"
DEVICE_HOST="${DEVICE_USER}@${DEVICE_IP}"

SOURCE_BINARY="/Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics/build/entos-diagnostics"
SOURCE_CONFIG="/Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics/config/providers.json"

TARGET_BINARY="/usr/bin/entos-diagnostics"
TARGET_CONFIG="/etc/entos-diagnostics/providers.json"

echo "════════════════════════════════════════════════════════════════"
echo "   Entos-Diagnostics Deployment Script"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Device:  $DEVICE_HOST"
echo "Binary:  $SOURCE_BINARY → $TARGET_BINARY"
echo "Config:  $SOURCE_CONFIG → $TARGET_CONFIG"
echo ""

# Step 1: Verify binary exists
echo "[1/5] Verifying binary exists..."
if [ ! -f "$SOURCE_BINARY" ]; then
    echo "❌ ERROR: Binary not found at $SOURCE_BINARY"
    echo "Build the project first: cd entos-diagnostics && cmake --build build"
    exit 1
fi
echo "✅ Binary found ($(ls -lh "$SOURCE_BINARY" | awk '{print $5}'))"
echo ""

# Step 2: Verify config exists
echo "[2/5] Verifying configuration file exists..."
if [ ! -f "$SOURCE_CONFIG" ]; then
    echo "❌ ERROR: Config not found at $SOURCE_CONFIG"
    exit 1
fi
echo "✅ Config found"
echo ""

# Step 3: Backup existing files
echo "[3/5] Creating backups on device..."
ssh "$DEVICE_HOST" << 'EOF'
TIMESTAMP=$(date +%s)
if [ -f /usr/bin/entos-diagnostics ]; then
    cp /usr/bin/entos-diagnostics /usr/bin/entos-diagnostics.backup.$TIMESTAMP
    echo "  ✅ Binary backup: /usr/bin/entos-diagnostics.backup.$TIMESTAMP"
fi
if [ -f /etc/entos-diagnostics/providers.json ]; then
    cp /etc/entos-diagnostics/providers.json /etc/entos-diagnostics/providers.json.backup.$TIMESTAMP
    echo "  ✅ Config backup: /etc/entos-diagnostics/providers.json.backup.$TIMESTAMP"
fi
EOF
echo ""

# Step 4: Deploy binary
echo "[4/5] Deploying binary..."
scp "$SOURCE_BINARY" "$DEVICE_HOST:$TARGET_BINARY"
ssh "$DEVICE_HOST" "chmod +x $TARGET_BINARY && ls -lh $TARGET_BINARY"
echo "✅ Binary deployed"
echo ""

# Step 5: Deploy configuration
echo "[5/5] Deploying configuration..."
scp "$SOURCE_CONFIG" "$DEVICE_HOST:$TARGET_CONFIG"
ssh "$DEVICE_HOST" "ls -lh $TARGET_CONFIG"
echo "✅ Config deployed"
echo ""

echo "════════════════════════════════════════════════════════════════"
echo "   Verifying Deployment"
echo "════════════════════════════════════════════════════════════════"
echo ""

# Verify help works
echo "[V1] Checking help command..."
ssh "$DEVICE_HOST" "entos-diagnostics --help" > /dev/null
echo "✅ Help command works"
echo ""

# Verify list works
echo "[V2] Checking list command..."
ssh "$DEVICE_HOST" "entos-diagnostics list" | head -5
echo ""

# Test package section
echo "[V3] Testing package manager section (main fix)..."
echo "Command: entos-diagnostics dumpsys --section package --format text"
echo ""
RESULT=$(ssh "$DEVICE_HOST" "entos-diagnostics dumpsys --section package --format text 2>&1" | head -20)

if echo "$RESULT" | grep -q "ERROR"; then
    echo "⚠️  WARNING: Section shows errors:"
    echo "$RESULT"
    echo ""
    echo "This might be expected if the service doesn't have data."
    echo "Key point: Should NOT see 'result violates configured response contract'"
else
    echo "✅ Package section works:"
    echo "$RESULT"
fi

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "   Deployment Complete ✅"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Next steps:"
echo "  1. Run full test: ssh root@<device> 'entos-diagnostics dumpsys --section package'"
echo "  2. Check JSON: ssh root@<device> 'entos-diagnostics dumpsys --section package --format json'"
echo "  3. Test other sections: runtime, window, resource"
echo ""
echo "Rollback (if needed):"
echo "  ssh root@<device> 'cp /usr/bin/entos-diagnostics.backup.* /usr/bin/entos-diagnostics'"
echo "  ssh root@<device> 'cp /etc/entos-diagnostics/providers.json.backup.* /etc/entos-diagnostics/providers.json'"
echo ""
