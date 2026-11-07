## Why

AI2.0 diagnostics are distributed across application managers, Thunder, SceneSet, the window manager, and platform configuration. A single read-only C++ utility should provide an appsservicectl-style snapshot while distinguishing observed data, inferred process modes, unavailable APIs, and collection failures.

## What Changes

- Introduce `entos-diagnostics dumpsys`, `plugins`, and `list` commands, with text and versioned JSON output, section selection, bounded collection, and meaningful exit codes.
- Discover all Thunder plugins and query verified read-only APIs through libcurl and JsonCpp; never activate plugins or invoke arbitrary advertised methods.
- Replace the process inspection behavior of `thunder-plugin-mode.sh` with native Linux procfs collection and literal callsign matching.
- Separate Thunder, application, and platform collectors behind a shared snapshot model and declarative, versioned provider registry.
- Include every section in the supplied `dumpsys.txt` coverage map. Report missing fields explicitly; do not claim API or output parity where the backend does not expose the data.
- Provide extension contracts for platform, systemd, GPU, input, application lifecycle, package, gateway, and compositor snapshots.
- Document API provenance, implementation sequence, cross-compilation, packaging, device acceptance, and the OpenSpec review/apply/archive workflow.

## Capabilities

### New Capabilities

- `diagnostics-snapshot`: CLI, common collector architecture, appsservicectl-style rendering, coverage reporting, and deployment.
- `thunder-diagnostics`: JSON-RPC transport, plugin discovery, read-only API coverage, and native plugin execution-mode evidence.
- `platform-application-diagnostics`: platform/configuration/process data and application/package/window snapshots, including explicit backend extension provisions.

### Modified Capabilities

None. Neighboring plugin APIs are not changed by this component-local proposal. Backend snapshot APIs require separately reviewed changes in their owning repositories.

## Impact

- Implementation and planning changes are scoped to `entos-diagnostics`; the seven neighboring repositories are evidence sources only.
- Runtime dependencies: C++17, libcurl, JsonCpp, Linux procfs/sysfs. Optional platform adapters may later use libsystemd, Wayland, and a YAML parser.
- No shell scripts, subprocess-based information retrieval, mandatory Thunder SDK linkage, plugin state changes, automatic privileged execution, or secret-bearing output.
- Full dumpsys parity depends on currently missing server-side introspection. Production rollout is gated on verified target interface bindings, field coverage, and Realtek device acceptance.
- A small process-mode C++ core and focused CTest are already present as a baseline; the CLI, transport, collectors, packaging, and device validation are not yet implemented.