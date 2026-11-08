#!/bin/bash
#
# AppManager Diagnostics Validation Script
# 
# This script helps validate AppManager integration:
# - Checks providers.json schema compliance
# - Generates mock Thunder responses for testing
# - Validates curl commands against target device
#
# Usage: ./validate_appmanager.sh [OPTIONS]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROVIDERS_JSON="${SCRIPT_DIR}/config/providers.json"
PROVIDERS_SCHEMA="${SCRIPT_DIR}/config/providers.schema.json"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Functions
print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# Check if jq is available
check_jq() {
    if ! command -v jq &> /dev/null; then
        print_error "jq is required. Install with: brew install jq (macOS) or apt-get install jq (Linux)"
        exit 1
    fi
    print_status "jq found"
}

# Validate JSON schema
validate_schema() {
    if ! command -v jsonschema &> /dev/null; then
        print_warning "jsonschema not found. Skipping schema validation. Install with: pip install jsonschema"
        return
    fi

    echo ""
    echo "Validating providers.json against schema..."
    if jsonschema -i "$PROVIDERS_JSON" "$PROVIDERS_SCHEMA" &> /dev/null; then
        print_status "providers.json schema validation passed"
    else
        print_error "providers.json schema validation failed"
        jsonschema -i "$PROVIDERS_JSON" "$PROVIDERS_SCHEMA"
        exit 1
    fi
}

# List all providers
list_providers() {
    echo ""
    echo "AppManager Diagnostic Providers:"
    echo "================================"
    
    jq -r '.providers[] | "\(.id) | \(.operation) | \(.section) | enabled=\(.enabled) | \(.verification)"' "$PROVIDERS_JSON" | \
    column -t -s '|'
}

# Check field mappings
check_field_mappings() {
    echo ""
    echo "Field Mappings:"
    echo "==============="
    
    jq -r '.providers[] | select(.fieldMappings != null) | "\(.id): \(.fieldMappings | keys | length) fields"' "$PROVIDERS_JSON"
}

# Generate mock responses for testing
generate_mock_responses() {
    echo ""
    echo "Generating mock Thunder responses..."
    
    mkdir -p "${SCRIPT_DIR}/tests/mocks"
    
    # Mock app.loaded response
    cat > "${SCRIPT_DIR}/tests/mocks/app_loaded.json" << 'EOF'
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
EOF
    print_status "Created app_loaded.json mock"
    
    # Mock app.installed response
    cat > "${SCRIPT_DIR}/tests/mocks/app_installed.json" << 'EOF'
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
EOF
    print_status "Created app_installed.json mock"
    
    # Mock package.list response
    cat > "${SCRIPT_DIR}/tests/mocks/package_list.json" << 'EOF'
{
  "success": true,
  "packages": [
    {
      "name": "com.netflix.nrdp",
      "version": "100.140.1.22110",
      "size": "52 MB",
      "installed": true,
      "type": "zip"
    },
    {
      "name": "com.bskyb.epgui",
      "version": "4.22.2",
      "size": "11 MB",
      "installed": true,
      "type": "zip"
    }
  ],
  "cache": {
    "used_bytes": 503316480,
    "limit_bytes": 104857600
  }
}
EOF
    print_status "Created package_list.json mock"
    
    # Mock resource.stats response
    cat > "${SCRIPT_DIR}/tests/mocks/resource_stats.json" << 'EOF'
{
  "success": true,
  "stats": [
    {
      "appId": "com.netflix.nrdp",
      "cpu": 5.2,
      "memory_bytes": 134217728,
      "timestamp": 1730977661
    },
    {
      "appId": "com.bskyb.epgui",
      "cpu": 3.1,
      "memory_bytes": 268435456,
      "timestamp": 1730977661
    }
  ],
  "total_cpu": 15.7,
  "total_memory_bytes": 2147483648
}
EOF
    print_status "Created resource_stats.json mock"
    
    # Mock window.clients response
    cat > "${SCRIPT_DIR}/tests/mocks/window_clients.json" << 'EOF'
{
  "success": true,
  "apps": [
    {
      "appId": "com.bskyb.epgui",
      "zOrder": 100,
      "focused": true,
      "visible": true
    },
    {
      "appId": "com.netflix.nrdp",
      "zOrder": 50,
      "focused": false,
      "visible": false
    }
  ],
  "focused_app_id": "com.bskyb.epgui"
}
EOF
    print_status "Created window_clients.json mock"
}

# Generate curl commands for manual testing
generate_curl_commands() {
    echo ""
    echo "Manual testing curl commands:"
    echo "=============================="
    
    cat > "${SCRIPT_DIR}/test_appmanager.sh" << 'EOF'
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
EOF
    
    chmod +x "${SCRIPT_DIR}/test_appmanager.sh"
    print_status "Created test_appmanager.sh script"
    echo "         Usage: ./test_appmanager.sh [host] [port]"
}

# Main menu
show_usage() {
    cat << EOF
Usage: $0 [COMMAND]

Commands:
  validate        Validate providers.json schema
  list            List all AppManager diagnostic providers
  fields          Show field mappings for each provider
  generate        Generate mock Thunder responses for testing
  curl            Generate curl test scripts
  all             Run all validations and generate test data
  help            Show this help message

Examples:
  $0 validate     # Check schema compliance
  $0 all          # Complete validation and test setup
  $0 curl         # Generate manual curl test commands

EOF
}

# Main
main() {
    cmd="${1:-all}"
    
    check_jq
    
    case "$cmd" in
        validate)
            validate_schema
            ;;
        list)
            list_providers
            ;;
        fields)
            check_field_mappings
            ;;
        generate)
            generate_mock_responses
            ;;
        curl)
            generate_curl_commands
            ;;
        all)
            validate_schema
            list_providers
            check_field_mappings
            generate_mock_responses
            generate_curl_commands
            echo ""
            print_status "All validations complete!"
            ;;
        help|--help|-h)
            show_usage
            ;;
        *)
            print_error "Unknown command: $cmd"
            show_usage
            exit 1
            ;;
    esac
}

main "$@"
