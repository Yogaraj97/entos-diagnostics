#!/bin/bash
# AppManager diagnostic queries via curl
# Usage: ./test_appmanager.sh [host] [port]

HOST="${1:-127.0.0.1}"
PORT="${2:-9998}"
URL="http://${HOST}:${PORT}/jsonrpc"

echo "Testing AppManager diagnostics against ${URL}"
echo ""

# Test app.loaded
echo "1. Testing app.loaded (getLoadedApps)..."
curl -s -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AppManager.1.getLoadedApps","params":{}}' "$URL" | jq .
echo ""

# Test app.installed
echo "2. Testing app.installed (getInstalledApps)..."
curl -s -d '{"jsonrpc":"2.0","id":2,"method":"org.rdk.AppManager.1.getInstalledApps","params":{}}' "$URL" | jq .
echo ""

# Test package.list
echo "3. Testing package.list (listPackages)..."
curl -s -d '{"jsonrpc":"2.0","id":3,"method":"org.rdk.AppPackageManager.1.listPackages","params":{}}' "$URL" | jq .
echo ""

# Fallback to org.rdk.PackageManager if AppPackageManager unavailable
echo "4. Testing package.list with fallback (PackageManager)..."
curl -s -d '{"jsonrpc":"2.0","id":4,"method":"org.rdk.PackageManager.1.listPackages","params":{}}' "$URL" | jq .
echo ""

# Test resource.stats
echo "5. Testing resource.stats (GetStats)..."
curl -s -d '{"jsonrpc":"2.0","id":5,"method":"org.rdk.ResourceMonitor.1.GetStats","params":{}}' "$URL" | jq .
echo ""

# Test window.clients
echo "6. Testing window.clients (getApps)..."
curl -s -d '{"jsonrpc":"2.0","id":6,"method":"org.rdk.RDKWindowManager.1.getApps","params":{}}' "$URL" | jq .
