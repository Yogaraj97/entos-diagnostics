# OpenSpec: entos-diagnostics Unified Diagnostics Module

## Overview
The entos-diagnostics module provides unified system diagnostics collection and display from Thunder APIs (RDK ecosystem) with support for multiple output formatters.

## Architecture

### Core Components

#### 1. Data Collection Engine
**Location**: `src/core/CollectionEngine.cpp`

Orchestrates collection of diagnostic data from Thunder service providers:
- Validates provider configurations
- Executes JSON-RPC calls to Thunder endpoints
- Validates responses against contracts
- Aggregates results into sections

#### 2. Provider System
**Location**: `config/providers.json`, `src/core/AppCollector.cpp`

Defines Thunder API providers and their field mappings:
- Each provider specifies: API endpoint, expected response type, field mappings
- Responses validated against section contracts
- Field extraction via path traversal (supports nested and array data)
- Provider overrides supported via `/opt/entos-diagnostics/providers.json.override`

#### 3. Output Formatters
**Location**: `src/application/DumpsysFormatter.cpp`

Specialized formatters for specific sections:
- `formatRuntimeManager()`: Loaded applications table
- `formatPackageManager()`: Installed packages table
- `formatResourceMonitor()`: System resource statistics
- `formatWindowManager()`: Window client information
- `formatPlatform()`: System platform info (kernel, memory, filesystems)

Generic formatter for other sections (array/object auto-detection)

### Data Flow

```
Thunder APIs
    ↓
AppCollector (JSON-RPC requests)
    ↓
Snapshot (Response validation & aggregation)
    ↓
DumpsysFormatter (Human-readable output)
    ↓
Console Display
```

## Supported Sections

| Section | Provider | Data | Format |
|---------|----------|------|--------|
| runtime | app.loaded | Loaded applications (appId, instanceId, state, pid) | Table |
| package | package.list | Installed packages (packageId, version, state, sizeKb) | Table |
| resource | resource.stats | System resources | Formatted text |
| window | window.clients | Window clients | Table |
| platform | platform.info | Kernel, memory, filesystems, uptime | Formatted sections |
| thunder | plugin.list | Thunder plugins | Table |
| config, dial, iui, etc. | Various | Section-specific data | Generic/formatted |

## Provider Configuration

### Structure
```json
{
  "id": "app.loaded",
  "section": "runtime",
  "api": "org.rdk.AppManager.getLoadedApps",
  "resultType": "array",
  "fieldMappings": [
    {
      "field": "apps",
      "path": []
    }
  ]
}
```

### Key Fields
- `id`: Unique provider identifier
- `section`: Target section (must exist in contracts)
- `api`: Thunder JSON-RPC method
- `resultType`: "object" or "array" (must match actual response)
- `fieldMappings`: How to extract and store data
  - `field`: Section field name (must be in section contract)
  - `path`: Navigation path (empty = root/entire response)

### Override Mechanism
Place provider modifications in `/opt/entos-diagnostics/providers.json.override`:
- Same structure as base `config/providers.json`
- Merged at runtime (overrides take precedence)
- Allows field additions/modifications without changing base config

## Output Formats

### Text Output (Default)
Human-readable format with sections, tables, and organized data:
```
PLATFORM:
    Status: partial

    KERNEL INFORMATION:
        System:        Linux
        Release:       5.4.186-1-6
        Machine:       aarch64

    SYSTEM UPTIME:
        0d 0h 19m (1179.47s)

    MEMORY INFORMATION:
        Total Memory:        2891 MB
        Available Memory:    1735 MB
        [...]
```

### JSON Output
Complete structured data with section metadata:
```json
{
  "sections": [
    {
      "id": "runtime",
      "title": "RUNTIME MANAGER",
      "status": "ok",
      "data": {...},
      "coverage": {...},
      "errors": [...]
    }
  ]
}
```

## Section Contracts

Located in `src/core/ProviderRegistry.cpp`:
- Define allowed fields per section
- Restrict provider field mappings to valid fields
- Ensure data structure consistency

Example:
```cpp
{"id": "runtime", "fields": {"apps", "lifecycle", "runtimeInfo", "sessions"}}
{"id": "package", "fields": {"cache", "packages", "installer", ...}}
```

## Command Interface

```bash
# Show help
entos-diagnostics

# List available sections
entos-diagnostics list

# Get specific section(s)
entos-diagnostics dumpsys --section runtime
entos-diagnostics dumpsys --section platform --section package

# Output formats
entos-diagnostics dumpsys --section runtime --format text
entos-diagnostics dumpsys --section runtime --format json

# Connection options
entos-diagnostics dumpsys --url http://custom.host:9998
entos-diagnostics dumpsys --token-file /path/to/token
entos-diagnostics dumpsys --timeout-ms 30000
```

## Array Handling

### Auto-Detection
When displaying array fields:
1. Check if array contains objects or primitives
2. Object arrays: Display as table with auto-detected columns
3. Primitive arrays: Display as bullet list
4. Empty arrays: Display as `[]`

### Memory Efficiency
- Streams processing when possible
- Validates response type before allocation
- Graceful handling of large result sets

## Error Handling

### Field Coverage
Each section tracks field status:
- `available`: Data successfully retrieved
- `unsupported`: Section doesn't support this field
- `unavailable`: Provider not enabled or data not returned

### Partial Results
Section status:
- `ok`: All fields available
- `partial`: Some fields available, some missing
- `error`: Section collection failed

### Provider Errors
Captured in section `errors` array:
- JSON-RPC errors
- Contract validation failures
- Response format mismatches

## Security

### Token Handling
- Read from file or environment variable
- Redacted in sanitized output
- Support for Bearer token format

### Data Sanitization
Sensitive fields automatically redacted:
- Passwords, secrets, tokens
- URLs with credentials
- Session IDs, device IDs
- MAC addresses

## Testing

### Mock Providers
Test fixtures in `tests/mocks/`:
- `app_loaded.json`: Runtime section data
- `package_list.json`: Package section data
- `platform.json`: Platform section data
- `window_clients.json`: Window section data

### Test Suites
- `DiagnosticsTests`: Provider validation, field extraction, formatting
- `TransportTests`: HTTP transport, JSON-RPC protocol

## Implementation Status

✅ **Core**
- Collection engine fully functional
- Provider system with override support
- Section contract validation
- Generic array handling

✅ **Formatters**
- Runtime manager (loaded apps table)
- Package manager (installed packages table)
- Resource monitor (stats display)
- Window manager (client information)
- Platform (kernel, memory, filesystems with status indication)

✅ **Output**
- Text formatter with proper alignment
- JSON formatter with full metadata
- Sanitization and redaction

✅ **CLI**
- Command parsing
- Section filtering
- Format selection
- Connection options
- Help display

## Build & Deployment

### Build
```bash
cd entos-diagnostics
cmake -B build
cmake --build build
```

### Binary
- Location: `build/entos-diagnostics`
- Size: ~703 KB
- Target: RDK/Linux (aarch64, x86_64)

### Installation
```bash
cp build/entos-diagnostics /usr/bin/
cp config/providers.json /etc/entos-diagnostics/
```

### Configuration
Base config: `/etc/entos-diagnostics/providers.json`
Overrides: `/opt/entos-diagnostics/providers.json.override`

## Design Principles

1. **Separation of Concerns**
   - Collection logic isolated from formatting
   - Formatters independent of data sources
   - Provider configuration separate from code

2. **Extensibility**
   - New sections added by updating contracts
   - New providers configured via JSON
   - Custom formatters added via new functions

3. **Reliability**
   - Contract validation at every step
   - Partial results preserved on error
   - Graceful degradation for unavailable providers

4. **User Experience**
   - Human-readable output (no raw JSON by default)
   - Clear error messages with reasons
   - Missing field explanations in status

## Future Enhancements

- Additional section formatters (dial, input, etc.)
- Filtering and sorting in output
- Performance metrics collection
- Historical tracking/trending
- Multi-format export (CSV, XML)
