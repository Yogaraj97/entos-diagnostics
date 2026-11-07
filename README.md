# EntOS Diagnostics

**A read-only C++17 diagnostic utility for unified snapshots of AI2.0 device state.**

Collects platform metrics, process information, and application/service state from Thunder plugins with deterministic output, automatic credential redaction, and deadline-aware collection. Supports text and JSON output, optional YAML configuration, and remote Thunder endpoints.

## Overview

`entos-diagnostics` provides a single executable that captures a complete snapshot of device health and application state without modifying any runtime configuration. It combines:

- **Native procfs collection**: Process detection, mode determination (Thunder vs. standalone)
- **Thunder plugin queries**: JSON-RPC over HTTP/TLS with configurable endpoints
- **Platform metrics**: Configuration, input handlers, system state (when available)
- **Application state**: Installed apps, lifecycles, runtime status (when verified)
- **Security-first design**: Automatic credential redaction, file permission checks, no shell helpers

**Status**: Host-side implementation complete (16/34 tasks). Live Thunder contracts and device deployment pending. All tests passing ✅

## Quick Start

### Build

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure  # 5/5 pass ✅
```

**Requirements**: CMake 3.16+, C++17, libcurl, JsonCpp. Optional: yaml-cpp (with `-DENTOS_ENABLE_YAML=ON`)

### Run

```bash
# List available data sources
./build/entos-diagnostics list

# Query Thunder plugin status
./build/entos-diagnostics plugins

# Full snapshot (text format)
./build/entos-diagnostics dumpsys

# Specific sections in JSON
./build/entos-diagnostics dumpsys --section platform --section config --format json

# Remote target with authentication
./build/entos-diagnostics dumpsys --url http://192.168.1.100:9998 --token-file ~/.thunder-token

# With configuration files (applied in order)
./build/entos-diagnostics dumpsys \
  --config-file /etc/entos-diagnostics/base.json \
  --config-file /etc/entos-diagnostics/local.yaml
```

**Exit codes**: 0 = complete, 1 = valid partial snapshot, 2 = invalid input/config

## Key Features

### ✅ Implemented

- **Read-only operations**: No state changes, no side effects
- **CLI interface**: `list`, `plugins`, `dumpsys` commands with flexible selectors
- **Output formats**: Human-readable text with tables; machine-readable JSON
- **Process correlation**: Detects Thunder plugin processes via procfs PPID
- **Secret redaction**: Credentials, tokens, URLs automatically removed from output
- **Deadline-aware**: Per-provider timeouts, graceful degradation under time pressure
- **Configuration management**: JSON/YAML file loading with merge policies (provisional)
- **HTTP transport**: Bounded libcurl integration with TLS/auth support
- **Schema validation**: Response validation against declared contracts
- **Installation**: Staged installation without privileged access

### 🚧 Pending (Blocked by External Dependencies)

| Feature | Blocker | Status |
|---------|---------|--------|
| **Thunder Contracts** | XiOne/ES1 SDK | Awaiting generated interfaces for AppManager, PackageManager, etc. |
| **Platform Metrics** | Device SDK | Awaiting cross-compile toolchain for uname, uptime, filesystem data |
| **Input Handler** | Device SDK | Awaiting input subsystem access for keymap inventory |
| **Optional Adapters** | Device SDK | Awaiting systemd, Wayland, Realtek GPU/thermal metrics |
| **Config Policies** | Product Owner | Awaiting JSON/YAML merge policy decision |
| **Yocto Recipe** | Product Team | Awaiting license decision + sysroot dependency mapping |
| **Device Acceptance** | Test Lab | Awaiting live XiOne/ES1 device access |

## Architecture

### Provider Pattern

All data collection follows a common `Provider` interface:

```cpp
class Provider {
  virtual SnapshotResult collect(SnapshotContext& ctx) = 0;
};
```

**Subclasses**:
- `ThunderProvider`: Queries plugins via JSON-RPC (HTTP transport)
- `ProcfsProvider`: Reads `/proc` for process and mode detection
- `ConfigProvider`: Loads and merges configuration files
- `PlatformProvider`: Collects system metrics (partial, requires device)

### Snapshot Model

Results are immutable, field-level tracked:

```cpp
struct FieldValue {
  std::string raw;           // Raw value
  FieldAvailability avail;   // present, unavailable, partial, error
  std::string source;        // "Thunder", "procfs", "config", ...
  std::string error;         // If unavailable: reason
};

class Snapshot {
  std::map<std::string, FieldValue> fields;
  // Renders as text table or JSON
};
```

### Threading & Cancellation

- One thread per provider (fan-out collection)
- Atomic `CancellationToken` for deadline enforcement
- Independent failure isolation: one provider error doesn't cascade
- Deterministic field ordering for reproducible output

## Documentation

**Start here**:
- [**design.md**](openspec/changes/add-unified-diagnostics/design.md) — C++ architecture, folder organization, provider contracts
- [**workflow.md**](openspec/changes/add-unified-diagnostics/workflow.md) — Build, test, cross-compile, deployment procedures
- [**tasks.md**](openspec/changes/add-unified-diagnostics/tasks.md) — 34-task implementation checklist with dependencies

**For context**:
- [**proposal.md**](openspec/changes/add-unified-diagnostics/proposal.md) — Scope, rationale, success criteria
- [**analysis.md**](openspec/changes/add-unified-diagnostics/analysis.md) — Seven-repository field evidence, legacy dumpsys comparison

## Testing

**5 CTest suites** (all passing ✅):

```
diagnostics_core ............ Provider interface, snapshot model, procfs
diagnostics_help ............ CLI parsing, command validation
diagnostics_list ............ Data source enumeration
diagnostics_transport ....... JSON-RPC protocol, HTTP handling, auth
diagnostics_cli ............. End-to-end scenarios, output safety
```

Run tests:
```bash
ctest --test-dir build --output-on-failure
```

**Test coverage**:
- Procfs edge cases (disappearing processes, permission denied)
- JSON-RPC malformed responses, ID mismatches, error handling
- Secret redaction (tokens, URLs, credentials never appear in output)
- Deadline exhaustion and partial results
- CLI option parsing and exit codes
- YAML and JSON configuration merging (when enabled)

## Installation

**Staged (development/testing)**:
```bash
DESTDIR=/tmp/entos-diagnostics-stage cmake --install build
# Installs to: /tmp/entos-diagnostics-stage/usr/bin/entos-diagnostics
```

**Cross-compilation (device deployment)**:
```bash
cmake -S . -B build-target \
  -DCMAKE_TOOLCHAIN_FILE="$ENTOS_SDK_TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_TESTING=OFF
cmake --build build-target
DESTDIR="$STAGING_DIR" cmake --install build-target
```

Then package via product Yocto layer (awaiting recipe approval).

## Usage Examples

### List All Providers
```bash
./build/entos-diagnostics list
```

**Output**:
```
config          CONFIG
dial            DIAL
iui             DYNAMIC IUI MANAGER
forwarded-launch FORWARDED LAUNCH MANAGER
input           INPUT HANDLER
package         PACKAGE MANAGER
...
```

### Check Thunder Plugin Status
```bash
./build/entos-diagnostics plugins
```

### Dump Platform Configuration
```bash
./build/entos-diagnostics dumpsys --section platform --format json
```

### Remote Diagnostics with Authentication
```bash
export THUNDER_URL=http://192.168.1.100:9998
export TOKEN='Authorization: Bearer <token>'
./build/entos-diagnostics dumpsys
```

Or use a token file (owner-only permissions):
```bash
./build/entos-diagnostics dumpsys --token-file /root/.thunder-auth --url http://target:9998
```

### Custom Configuration
```bash
# JSON overrides
./build/entos-diagnostics dumpsys \
  --config-file /etc/entos-diagnostics/base.json \
  --config-file /etc/entos-diagnostics/local.yaml

# Specific sections
./build/entos-diagnostics dumpsys \
  --section platform \
  --section input \
  --section config \
  --format json
```

## Security Considerations

✅ **Implemented**:
- No hardcoded credentials
- Token file permission checks (must be owner-readable only)
- Automatic redaction of secrets from all output and error messages
- No subprocess/shell execution
- Read-only operations (no configuration modification)
- TLS support for remote endpoints

⚠️ **Important**:
- Never pass credentials positionally: `./build/entos-diagnostics --token "secret"` ❌
- Always use: `--token-file` or `TOKEN` environment variable ✅
- Staged install location must not contain token files ✅

## Configuration Files

Supports JSON and YAML (optional, with `-DENTOS_ENABLE_YAML=ON`):

**JSON example** (`/etc/entos-diagnostics/config.json`):
```json
{
  "thunder": {
    "url": "http://127.0.0.1:9998",
    "timeout_ms": 5000
  },
  "profiles": ["platform", "input"],
  "output": {
    "redact_secrets": true,
    "format": "json"
  }
}
```

**YAML example** (`/etc/entos-diagnostics/config.yaml`):
```yaml
thunder:
  url: http://127.0.0.1:9998
  timeout_ms: 5000
profiles:
  - platform
  - input
output:
  redact_secrets: true
  format: json
```

Configuration values are **declared state only** (read from files), not effective runtime state.

## Development

### IDE Setup (VS Code)

1. Install C/C++ extension (ms-vscode.cpptools)
2. Configure in settings: `"C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json"`
3. Full IntelliSense, debugging, and symbol navigation now available

See [VSCODE_SETUP.md](openspec/changes/add-unified-diagnostics/VSCODE_SETUP.md) for details.

### Adding a New Provider

1. Create class in `src/collectors/` inheriting from `Provider`
2. Implement `SnapshotResult collect(SnapshotContext&)`
3. Register in `ProviderRegistry::registerProviders()`
4. Add tests in `tests/` (fixture-based, no live endpoints)
5. Run: `ctest --test-dir build --output-on-failure`
6. Update provider list in `config/providers.json`

Example provider structure:
```cpp
class MyProvider : public Provider {
 public:
  SnapshotResult collect(SnapshotContext& ctx) override {
    // Check cancellation
    if (ctx.isCancelled()) return SnapshotResult::cancelled();
    
    // Collect data within deadline
    // Return result with field-level availability
  }
};
```

### Before Committing

```bash
# Ensure all tests pass
ctest --test-dir build --output-on-failure

# Check for warnings
cmake --build build -- --verbose

# Verify installation
DESTDIR=/tmp/test-stage cmake --install build
ls -la /tmp/test-stage/usr/bin/
```

## Release Checklist

This change cannot be archived until:

- [ ] 34 of 34 tasks complete
- [ ] Thunder SDK provides generated COM interfaces (XiOne/ES1)
- [ ] Sanitized live Thunder responses verified against contracts
- [ ] Cross-build tested with product SDK
- [ ] Device acceptance testing complete (appsservicectl comparison)
- [ ] Memory and elapsed-time budgets verified on target
- [ ] Yocto recipe approved with license decision
- [ ] Package installation and rollback verified
- [ ] OpenSpec validation passes (strict mode)
- [ ] All documentation updated

**Current status**: 16 of 34 tasks complete. Awaiting external dependencies (SDK, device, product decisions).

## Support & Next Steps

**For developers**:
- See [design.md](openspec/changes/add-unified-diagnostics/design.md) for architecture
- See [tasks.md](openspec/changes/add-unified-diagnostics/tasks.md) for what to work on next
- Check [workflow.md](openspec/changes/add-unified-diagnostics/workflow.md) for build/test procedures

**For product integration**:
- Thunder SDK and generated interfaces required for app/package adapters
- Device deployment requires cross-compile toolchain and Yocto recipe
- Acceptance testing requires live device access

**Critical constraints**:
- ✋ Do NOT enable disabled profiles without verified Thunder responses
- ✋ Do NOT modify neighboring repositories (AppManager, PackageManager, etc.)
- ✋ Do NOT claim OpenSpec validation proves runtime correctness
- ✅ Always use fixtures for testing (no live API access in CI)
- ✅ All changes must pass `ctest --output-on-failure` before commit

## License

Awaiting product team decision for Yocto recipe integration. See LICENSE file for current terms.

## Contributing

This is an OpenSpec-tracked change. See [proposal.md](openspec/changes/add-unified-diagnostics/proposal.md) and [tasks.md](openspec/changes/add-unified-diagnostics/tasks.md) for implementation scope and constraints.