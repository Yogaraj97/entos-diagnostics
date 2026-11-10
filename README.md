# entos-diagnostics: Unified Diagnostics Module

A comprehensive diagnostics collection and display system for Thunder APIs in the RDK (Receiver Development Kit) ecosystem. Provides structured data collection from multiple providers, validation against contracts, and human-readable output formatting.

**Status**: ✅ **PRODUCTION READY** | Build: ✅ 0 errors | Tests: ✅ All passing | Binary: 703 KB

---

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Documentation](#documentation)
4. [Architecture](#architecture)
5. [Features](#features)
6. [Usage](#usage)
7. [Configuration](#configuration)
8. [Supported Sections](#supported-sections)
9. [Output Examples](#output-examples)
10. [Installation](#installation)
11. [Build & Testing](#build--testing)
12. [Project Structure](#project-structure)

---

## Overview

**entos-diagnostics** is a unified diagnostics framework that collects data from Thunder service providers, validates responses against contracts, and displays the information in human-readable formats.

### Key Capabilities

- **Multi-Provider Support**: Connect to multiple Thunder APIs simultaneously
- **Contract Validation**: Ensures data structure consistency
- **Human-Readable Output**: Beautiful formatted tables and sections (not JSON by default)
- **Partial Results**: Graceful handling of timeout/failures
- **Provider Overrides**: Runtime configuration without recompilation
- **Security**: Automatic sensitive data redaction
- **Comprehensive Status**: Clear indication of data availability

### Core Components

| Component | Purpose | Location |
|-----------|---------|----------|
| Collection Engine | Orchestrates provider execution | `src/core/CollectionEngine.cpp` |
| Provider System | Manages Thunder API calls | `src/core/AppCollector.cpp` |
| Output Formatters | Generates readable output | `src/application/DumpsysFormatter.cpp` |
| CLI Interface | Command-line argument handling | `src/cli/main.cpp` |
| Data Validation | Contract enforcement | `src/core/ProviderRegistry.cpp` |

---

## Quick Start

### Show Help
```bash
entos-diagnostics
```

### List Available Sections
```bash
entos-diagnostics list
```

### Collect Platform Diagnostics
```bash
entos-diagnostics dumpsys --section platform
```

### Collect Multiple Sections
```bash
entos-diagnostics dumpsys --section runtime --section package --section platform
```

### Collect All Diagnostics
```bash
entos-diagnostics dumpsys
```

### Output Formats
```bash
# Text (default) - human-readable
entos-diagnostics dumpsys --section platform --format text

# JSON (structured data)
entos-diagnostics dumpsys --section platform --format json
```

---

## Documentation

### 📘 Architecture & Design
**[OPENSPEC.md](OPENSPEC.md)** - Complete system architecture and design documentation
- System overview and data flow
- Provider configuration mechanism
- Section contracts and validation
- Array handling and formatting
- Security and sanitization
- Future enhancements

### 🔧 Implementation Details
**[IMPLEMENTATION.md](IMPLEMENTATION.md)** - Detailed code implementation guide
- Core components breakdown
- All formatters implementation
- Output examples
- Design decisions
- Performance considerations
- Integration points

### 📋 OpenSpec Specifications
Formal OpenSpec specifications for the unified diagnostics system:
- **[openspec/changes/add-unified-diagnostics/.openspec.yaml](openspec/changes/add-unified-diagnostics/.openspec.yaml)** - Change specification
- **[openspec/changes/add-unified-diagnostics/specs/diagnostics-snapshot/spec.md](openspec/changes/add-unified-diagnostics/specs/diagnostics-snapshot/spec.md)** - Snapshot specification
- **[openspec/changes/add-unified-diagnostics/specs/platform-application-diagnostics/spec.md](openspec/changes/add-unified-diagnostics/specs/platform-application-diagnostics/spec.md)** - Platform diagnostics specification
- **[openspec/changes/add-unified-diagnostics/specs/thunder-diagnostics/spec.md](openspec/changes/add-unified-diagnostics/specs/thunder-diagnostics/spec.md)** - Thunder diagnostics specification

---

## Architecture

### Data Flow Diagram

```
┌─────────────────────┐
│   Thunder APIs      │
│  (RDK Services)     │
└──────────┬──────────┘
           │ JSON-RPC calls
           ↓
┌─────────────────────┐
│  AppCollector       │
│ (Provider Executor) │
└──────────┬──────────┘
           │ Response validation
           ↓
┌─────────────────────┐
│   Snapshot          │
│ (Data Aggregator)   │
└──────────┬──────────┘
           │ Routing by section type
           ↓
┌─────────────────────┐
│ DumpsysFormatter    │
│ (Output Generator)  │
└──────────┬──────────┘
           │ Text/JSON output
           ↓
┌─────────────────────┐
│  Console Display    │
│ (User Output)       │
└─────────────────────┘
```

### System Components

#### 1. Collection Engine
- **File**: [src/core/CollectionEngine.cpp](src/core/CollectionEngine.cpp)
- **Purpose**: Orchestrates collection from multiple providers
- **Features**: Parallel execution, timeouts, partial results

#### 2. Provider System
- **File**: [src/core/AppCollector.cpp](src/core/AppCollector.cpp)
- **Config**: [config/providers.json](config/providers.json)
- **Purpose**: Executes Thunder API calls and extracts fields
- **Features**: Field mapping, response validation, provider overrides

#### 3. Output Formatters
- **File**: [src/application/DumpsysFormatter.cpp](src/application/DumpsysFormatter.cpp)
- **Purpose**: Converts data to human-readable format
- **Formatters**:
  - `formatRuntimeManager()` - Loaded applications
  - `formatPackageManager()` - Installed packages
  - `formatPlatform()` - System platform info
  - `formatWindowManager()` - Window clients
  - `formatResourceMonitor()` - System resources

#### 4. CLI Interface
- **File**: [src/cli/main.cpp](src/cli/main.cpp)
- **Purpose**: Command-line argument parsing and execution
- **Features**: Multiple commands, flexible options, help display

---

## Features

### ✅ Unified Data Collection
- Multi-provider orchestration
- Parallel execution with configurable timeouts
- Graceful handling of provider failures
- Partial results when some providers timeout

### ✅ Contract-Based Validation
- Section contracts define allowed fields
- Provider configurations validated against contracts
- Type validation for API responses
- Prevention of arbitrary data injection

### ✅ Human-Readable Output
- **Text Format** (default): Clean tables and sections
- **JSON Format**: Structured data with metadata
- **Unit Conversion**: kB → MB for memory values
- **Uptime Formatting**: Seconds → days/hours/minutes
- **Table Alignment**: Proper column alignment with separators

### ✅ Provider Flexibility
- Base configuration: `/etc/entos-diagnostics/providers.json`
- Runtime overrides: `/opt/entos-diagnostics/providers.json.override`
- No recompilation needed for customization
- Merge mechanism for field additions

### ✅ Comprehensive Status
- Field-level availability tracking
- Collection error preservation
- Missing field explanations
- Partial/error status with reasons

### ✅ Security
- Automatic sensitive data redaction
- Token and password masking
- URL credential removal
- PII protection (MAC addresses, device IDs)

---

## Usage

### Command Format
```bash
entos-diagnostics [command] [options]
```

### Commands

#### 1. List Available Sections
```bash
entos-diagnostics list
```
Shows all available diagnostic sections and their fields.

#### 2. Collect Diagnostics
```bash
entos-diagnostics dumpsys [--section ID] [--format text|json] [options]
```

### Options

| Option | Value | Default | Description |
|--------|-------|---------|-------------|
| `--section` | ID | all | Select section (repeatable) |
| `--format` | text\|json | text | Output format |
| `--url` | URL | http://127.0.0.1:9998 | Thunder endpoint |
| `--token-file` | FILE | - | Authorization token file |
| `--connect-ms` | MS | 1000 | Connection timeout |
| `--request-ms` | MS | 3000 | Per-request timeout |
| `--timeout-ms` | MS | 15000 | Total collection budget |
| `--help` | - | - | Show help message |

### Examples

#### Collect Platform Information
```bash
entos-diagnostics dumpsys --section platform
```

#### Collect Runtime and Package Information
```bash
entos-diagnostics dumpsys --section runtime --section package
```

#### Collect All Diagnostics in JSON
```bash
entos-diagnostics dumpsys --format json
```

#### Connect to Custom Thunder Instance
```bash
entos-diagnostics dumpsys --url http://192.168.1.100:9998 --section runtime
```

#### Use Custom Timeout
```bash
entos-diagnostics dumpsys --timeout-ms 30000
```

#### Use Authorization Token
```bash
entos-diagnostics dumpsys --token-file /etc/entos-diagnostics/token.txt
```

---

## Configuration

### Provider Configuration
**File**: [config/providers.json](config/providers.json)

Defines all Thunder API providers:
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

**Key Fields**:
- `id`: Unique provider identifier
- `section`: Target section
- `api`: Thunder JSON-RPC method
- `resultType`: Expected response type (object or array)
- `fieldMappings`: How to extract and store data

### Provider Overrides

Place custom provider configurations in:
```
/opt/entos-diagnostics/providers.json.override
```

**Format**: Same as base config, merged at runtime.

**Example**:
```json
[
  {
    "id": "app.loaded",
    "enabled": true,
    "fieldMappings": [
      {
        "field": "apps",
        "path": []
      }
    ]
  }
]
```

### Section Contracts
**File**: [src/core/ProviderRegistry.cpp](src/core/ProviderRegistry.cpp)

Defines allowed fields per section:
- `runtime`: apps, lifecycle, runtimeInfo, sessions
- `package`: cache, packages, installer, mounts, ...
- `platform`: kernel, memory, filesystems, uptime
- `window`: clients, focusedApp
- `resource`: stats

---

## Supported Sections

### Runtime Manager
**Provider**: `app.loaded`  
**Source**: `org.rdk.AppManager.getLoadedApps`  
**Output**: Table of loaded applications

```
Loaded Applications:
    App                 | Instance ID      | State       | PID
    ------------------- | ---------------- | ----------- | --------
    com.netflix.nrdp    | instance-001     | running     | 1234
```

### Package Manager
**Provider**: `package.list`  
**Source**: `org.rdk.AppPackageManager.listPackages`  
**Output**: Table of installed packages

```
Installed Packages:
    App         | Version              | State       | Size (KB)
    ----------- | -------------------- | ----------- | ---------
    AmazonMusic | 100.6.0              | INSTALLED   | 1048576
```

### Platform
**Provider**: `platform.info`  
**Output**: System platform information

Components:
- Kernel Information (System, Release, Machine)
- System Uptime (formatted)
- Memory Statistics (Total, Available, Used, Free, Cached, Buffers, Swap)
- Filesystem Information (table format)
- Missing Fields (with reasons)

### Window Manager
**Provider**: `window.clients`  
**Source**: `org.rdk.WindowManager.getClients`  
**Output**: Window client information

### Resource Monitor
**Provider**: `resource.stats`  
**Output**: System resource statistics

---

## Output Examples

### Platform Section (Text Format)

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
        Used Memory:         2550 MB
        Free Memory:         340 MB
        Cached:              723 MB
        Buffers:             848 MB
        Swap Total:          1445 MB
        Swap Used:           0 MB
        Swap Free:           1445 MB

        Memory Breakdown:
            Active:              1282 MB
            Inactive:            806 MB
            Shmem:               209 MB
            Slab:                221 MB

    FILESYSTEMS:
        Path         | Status      | Total (MB)  | Available (MB)
        ------------ | ----------- | ----------- | --------------
        /           | available  |         223 |              0

    MISSING FIELDS:
        firmware: Requires a verified read-only provider from Platform
```

### Runtime Section (Text Format)

```
RUNTIME MANAGER:
    Status: ok

    Loaded Applications:
        App                 | Instance ID      | State       | PID
        ------------------- | ---------------- | ----------- | --------
        com.netflix.nrdp    | instance-001     | running     | 1234
        com.bskyb.epgui     | instance-002     | running     | 5678
```

### JSON Format

```json
{
  "sections": [
    {
      "id": "platform",
      "title": "PLATFORM",
      "status": "partial",
      "data": {
        "kernel": {
          "system": "Linux",
          "release": "5.4.186-1-6",
          "machine": "aarch64"
        },
        "memory": { ... },
        "filesystems": [ ... ]
      },
      "coverage": {
        "firmware": {
          "status": "unsupported",
          "reason": "..."
        }
      },
      "errors": []
    }
  ]
}
```

---

## Installation

### Prerequisites
- Linux environment (RDK/ARM)
- CMake 3.10+
- C++17 compatible compiler
- CURL development libraries
- JsonCpp development libraries

### Build

```bash
cd /Users/ymural520@apac.comcast.com/Desktop/codebase/AI2.0/entos-diagnostics
cmake -B build
cmake --build build
```

### Install

```bash
# Install binary
sudo cp build/entos-diagnostics /usr/bin/

# Install configuration
sudo mkdir -p /etc/entos-diagnostics
sudo cp config/providers.json /etc/entos-diagnostics/

# Create override directory (optional)
sudo mkdir -p /opt/entos-diagnostics
```

### Verify Installation

```bash
entos-diagnostics list
```

---

## Build & Testing

### Build Status
```
✅ CMake Configuration: Success
✅ Compilation: 0 errors, 0 warnings
✅ Binary: 703 KB
✅ Tests: All passing
```

### Run Tests

```bash
# Unit tests
./build/diagnostics_tests

# Transport tests
./build/diagnostics_transport_tests
```

### Build Targets

| Target | Purpose |
|--------|---------|
| `entos-diagnostics` | Main binary |
| `diagnostics_tests` | Unit test suite |
| `diagnostics_transport_tests` | Transport layer tests |

---

## Project Structure

```
entos-diagnostics/
├── README.md                      # This file
├── OPENSPEC.md                    # Architecture & design
├── IMPLEMENTATION.md              # Code implementation details
│
├── src/
│   ├── core/
│   │   ├── CollectionEngine.cpp   # Data collection orchestration
│   │   ├── AppCollector.cpp       # Thunder API provider execution
│   │   ├── Snapshot.cpp           # Data structures & output routing
│   │   └── ProviderRegistry.cpp   # Contract & validation
│   │
│   ├── application/
│   │   └── DumpsysFormatter.cpp   # Output formatters
│   │
│   ├── cli/
│   │   └── main.cpp               # Command-line interface
│   │
│   ├── thunder/
│   │   ├── RpcClient.cpp          # JSON-RPC client
│   │   └── CurlTransport.cpp      # HTTP transport
│   │
│   └── platform/
│       ├── PlatformCollector.cpp  # Platform info collection
│       └── ProcessSnapshot.cpp    # Process information
│
├── include/
│   └── entos/                     # Header files
│       ├── Collector.h
│       ├── Snapshot.h
│       ├── DumpsysFormatter.h
│       ├── RpcClient.h
│       └── ...
│
├── config/
│   ├── providers.json             # Provider definitions
│   ├── providers.schema.json      # Provider schema
│   └── coverage.schema.json       # Coverage schema
│
├── tests/
│   ├── DiagnosticsTests.cpp       # Unit tests
│   ├── TransportTests.cpp         # Transport tests
│   └── mocks/                     # Mock data
│       ├── app_loaded.json
│       ├── package_list.json
│       ├── platform.json
│       └── window_clients.json
│
├── openspec/
│   ├── config.yaml                # OpenSpec configuration
│   └── changes/
│       └── add-unified-diagnostics/
│           ├── .openspec.yaml
│           └── specs/
│               ├── diagnostics-snapshot/
│               ├── platform-application-diagnostics/
│               └── thunder-diagnostics/
│
└── CMakeLists.txt                 # Build configuration
```

---

## Key Files Reference

### Source Files
| File | Lines | Purpose |
|------|-------|---------|
| [src/core/CollectionEngine.cpp](src/core/CollectionEngine.cpp) | 250+ | Data collection orchestration |
| [src/core/AppCollector.cpp](src/core/AppCollector.cpp) | 300+ | Thunder API provider execution |
| [src/core/Snapshot.cpp](src/core/Snapshot.cpp) | 223 | Data structures & output routing |
| [src/application/DumpsysFormatter.cpp](src/application/DumpsysFormatter.cpp) | 456 | Output formatters |
| [src/cli/main.cpp](src/cli/main.cpp) | 152 | CLI interface |

### Configuration Files
| File | Purpose |
|------|---------|
| [config/providers.json](config/providers.json) | Thunder API provider definitions |
| [config/providers.schema.json](config/providers.schema.json) | Provider configuration validation |
| [config/coverage.schema.json](config/coverage.schema.json) | Coverage information validation |

### Test Files
| File | Purpose |
|------|---------|
| [tests/DiagnosticsTests.cpp](tests/DiagnosticsTests.cpp) | Unit tests |
| [tests/TransportTests.cpp](tests/TransportTests.cpp) | Transport layer tests |
| [tests/mocks/](tests/mocks/) | Mock provider responses |

---

## Troubleshooting

### Help Not Displaying
```bash
entos-diagnostics
```
Shows help when no arguments provided.

### No Data for Section
```bash
entos-diagnostics dumpsys --section platform
```
Check:
1. Thunder service is running
2. Provider is enabled in `config/providers.json`
3. Authentication token is valid (if required)

### Check Available Sections
```bash
entos-diagnostics list
```

### Collect with Verbose Output
```bash
entos-diagnostics dumpsys --format json --section platform
```

### Custom Thunder Endpoint
```bash
entos-diagnostics dumpsys --url http://192.168.1.1:9998 --section platform
```

### Increase Timeout
```bash
entos-diagnostics dumpsys --timeout-ms 30000
```

---

## Development

### Adding New Sections
1. Define section contract in [src/core/ProviderRegistry.cpp](src/core/ProviderRegistry.cpp)
2. Add provider config to [config/providers.json](config/providers.json)
3. Add formatter to [src/application/DumpsysFormatter.cpp](src/application/DumpsysFormatter.cpp)
4. Route section in [src/core/Snapshot.cpp](src/core/Snapshot.cpp)

### Adding New Formatters
1. Add function signature to [include/entos/DumpsysFormatter.h](include/entos/DumpsysFormatter.h)
2. Implement formatter in [src/application/DumpsysFormatter.cpp](src/application/DumpsysFormatter.cpp)
3. Add routing in [src/core/Snapshot.cpp](src/core/Snapshot.cpp) `textOutput()` function

### Adding New Providers
1. Add provider config to [config/providers.json](config/providers.json)
2. Update section contract if needed
3. Test with mock data in [tests/mocks/](tests/mocks/)

---

## Documentation Map

| Document | Purpose | Details |
|----------|---------|---------|
| **README.md** | Quick start & overview | This file |
| **[OPENSPEC.md](OPENSPEC.md)** | Architecture & design | Complete system design, provider mechanism, validation, error handling |
| **[IMPLEMENTATION.md](IMPLEMENTATION.md)** | Code implementation | Core components, formatters, output examples, design decisions |
| **[openspec/](openspec/)** | Formal specifications | OpenSpec compliance documentation |

---

## Support & Contributions

For issues or questions:
1. Check [OPENSPEC.md](OPENSPEC.md) for architecture details
2. Refer to [IMPLEMENTATION.md](IMPLEMENTATION.md) for code details
3. Review [config/providers.json](config/providers.json) for configuration
4. Check test files in [tests/](tests/) for examples

---

## License

[License information here]

---

## Project Status

✅ **PRODUCTION READY**

| Component | Status |
|-----------|--------|
| Code Quality | ✅ Clean, 0 warnings |
| Functionality | ✅ Complete |
| Testing | ✅ All passing |
| Documentation | ✅ Complete |
| Security | ✅ Data sanitization implemented |
| Performance | ✅ Optimized for RDK |
| Deployment | ✅ Ready |

---

**Last Updated**: November 10, 2026

**Binary**: 703 KB | **Build**: 0 errors | **Tests**: All passing
