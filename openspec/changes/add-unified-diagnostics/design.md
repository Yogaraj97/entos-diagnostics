## Context

See [proposal.md](proposal.md) for motivation and [analysis.md](analysis.md) for repository evidence and coverage gaps. At proposal creation the implementation was only a C++17 process snapshot/mode core. Apply now provides a runnable host CLI, transport, collector engine, initial native collectors, response mappings and five CTest suites. The checklist records incomplete adapters and target gates. The component uses the OpenSpec `spec-driven` schema.

The neighboring repositories mix Thunder JSON-RPC bindings, COM-RPC-only interfaces, application-service commands, systemd services, and Wayland protocols. Generated Thunder interface headers are external to this workspace. Interface declarations alone do not prove JSON-RPC spelling or wire shape. Legacy ASAppManager `Dump` is debug-only. The utility cannot manufacture private backend state.

## Goals / Non-Goals

**Goals:**
- Keep data retrieval, orchestration, normalization, and presentation independent.
- Allow application and platform collectors to work when Thunder is unavailable.
- Preserve field provenance and truthful partial results; make missing backend contracts easy to assign.
- Keep target-side runtime C++ only, with dependency-injected transport and filesystem roots for testing.

**Non-Goals:**
- Running shell utilities, parsing `ps` output, or implementing an SSH/remote command runner.
- Activating/deactivating plugins, launching apps, repairing devices, downloading packages, or changing settings.
- Shipping invented backend APIs or treating logging-context configuration as diagnostics.
- Claiming an atomic system snapshot: independently collected timestamps and elapsed time are exposed.
- Linking every service implementation into the utility or requiring root for normal use.

## Decisions

### 1. Executable plus small collector library

Build a standalone `entos-diagnostics` executable and a testable `diagnostics_core` library, rather than a new Thunder plugin. A plugin would depend on the framework being healthy during diagnosis and needlessly expand deployment coupling. The baseline process library remains reusable; transport dependencies are introduced only during apply.

Target folder architecture, not a claim that these files already exist:

```text
entos-diagnostics/
  CMakeLists.txt
  README.md
  include/entos/
    ProcessSnapshot.h             # present baseline
    Snapshot.h                    # field/status/source models
    Collector.h                   # common read-only provider contract
    Context.h                     # target, deadlines, cancellation
    RpcClient.h                   # injectable transport contract
    ProviderRegistry.h            # reviewed bindings and field coverage
  src/
    cli/main.cpp                  # parse commands/options only
    core/Snapshot.cpp
    core/CollectionEngine.cpp     # dependencies, budget, partial results
    core/ProviderRegistry.cpp
    core/Redaction.cpp
    thunder/CurlTransport.cpp      # bounded HTTP and authentication
    thunder/RpcClient.cpp          # JSON-RPC envelope validation
    thunder/PluginCollector.cpp
    platform/ProcessSnapshot.cpp  # present baseline
    platform/PlatformCollector.cpp
    platform/ConfigurationCollector.cpp
    platform/InputCollector.cpp
    platform/SystemdCollector.cpp # optional native sd-bus
    platform/RealtekCollector.cpp # optional GPU/thermal provider
    application/AppCollector.cpp
    application/PackageCollector.cpp
    application/WindowCollector.cpp
    application/GatewayCollector.cpp
    application/StartupCollector.cpp
    adapters/WaylandSnapshot.cpp   # optional native read-only protocol
    output/TextRenderer.cpp
    output/JsonRenderer.cpp
  config/
    diagnostics.json              # deadlines, selectors, allowlisted files
    providers.json                # schema/version/read-only API registry
    coverage.json                 # dumpsys field contract and owners
  tests/
    DiagnosticsTests.cpp          # present baseline; reuse for focused units
    fixtures/                     # sanitized RPC, procfs, and sysfs cases
  packaging/
    entos-diagnostics.bb          # reviewed Yocto template during apply
  openspec/
    config.yaml
    specs/                        # populated by sync/archive, not copied early
    changes/add-unified-diagnostics/
      proposal.md
      design.md
      analysis.md
      workflow.md
      tasks.md
      specs/<capability>/spec.md
```

Do not create empty implementation directories or placeholder collectors. Add each file when a reviewed task implements it; combine tiny adapters where doing so removes needless indirection.

### 2. Shared snapshot model

`ICollector::collect(const CollectionContext&) -> SectionSnapshot` is the planned owning abstraction. Collectors declare stable IDs, dependencies, locality, required fields, and optional features. They return data instead of printing. Renderers never perform network or filesystem calls.

The model includes `Snapshot { schemaVersion, target, startedAt, elapsedMs, sections }`, `SectionSnapshot { id, title, status, collectedAt, source, data, coverage, errors }`, and `FieldAvailability { path, status, reason, provider, source, collectedAt }`. Field statuses distinguish available, unsupported, not-implemented, unavailable, permission-denied, and timeout. Aggregate section status follows required coverage; a successful getter does not make a whole legacy section complete.

Tables are section-specific normalized views, not arbitrary JSON dumps. Preserve structured data in JSON and list unmapped response fields separately when useful. Preserve chronological loaded-app order when it is semantically defined; sort plugin identities and unordered inventories consistently.

### 3. Reviewed provider registry, not arbitrary RPC execution

Use JsonCpp for JSON parsing, matching the existing application manager. Use libcurl for HTTP; neither `curl` executable nor a shell is invoked. Prefer installed CMake/package-manager dependencies over network fetching during configure. `providers.json` contains schema version, candidate callsigns, interface version, exact operation name, immutable read-only classification, parameter schema, response schema, embedded-JSON paths, section/field mapping, and evidence status.

Bindings proven by local JSON-RPC tests can be enabled after fixtures confirm response contracts. Getter declarations with missing generated bindings remain provisional and disabled. On unknown method, try only another reviewed candidate. Do not silently fall back on authentication, timeout, malformed results, or application errors. All discovered plugins are inventoried; no advertised method is executed merely because it exists.

Example registry shape (design example, not a deployed configuration):

```json
{
  "schemaVersion": 1,
  "providers": [{
    "id": "app.loaded",
    "section": "runtime",
    "callsigns": ["org.rdk.AppManager"],
    "interfaceVersion": "1",
    "operation": "getLoadedApps",
    "readOnly": true,
    "params": {},
    "verification": "rpc-name-verified-response-pending",
    "enabled": false,
    "evidence": "entservices-appmanagers/Tests/L1Tests/tests/test_AppManager.cpp"
  }]
}
```

Version 1 in this example is an explicit provisional profile setting, not evidence that all deployed interfaces use version 1. Verify actual versions against the target SDK and device.

### 4. Transport and deadline handling

Normalize the configured base URL and POST to its JSON-RPC endpoint. Default to loopback. Reject userinfo credentials and non-HTTP(S) schemes. Use libcurl's own structured options, a RAII header list, certificate verification, connection timeout, request timeout, maximum body size, and cancellation callback. Use a bounded scheduler; start with sequential independent calls and add limited concurrency only if target measurements justify it.

Proposed defaults: 1-second connect timeout, 3-second request timeout, 15-second total budget, 2 MiB maximum response per call, and 64 applications/clients for fan-out. Every call uses the smaller of request timeout and remaining budget; reaching a count limit reports truncated coverage. Make limits configurable within validated bounds.

Validate object envelope, `jsonrpc == "2.0"`, exact matching request ID, and result/error exclusivity. Preserve safe numeric error codes; sanitize messages before storage. Parse supported Controller result shapes only through named version adapters backed by fixtures. Do not recursively search arbitrary JSON for callsigns. Credentials come from a protected file or inherited environment; reject CR/LF and never include tokens in URLs, process details, or logs.

### 5. Native process-mode translation

The supplied script's intent is retained, but its ambiguous assumptions are corrected:

| Script behavior | C++ design |
| --- | --- |
| Shell `curl` and split authorization string | libcurl POST with one protected header |
| `.plugins` assumed at top level | validated JSON-RPC result adapter |
| jq/grep/sed JSON fallback | one structured JSON parser |
| `ps` snapshot and regex callsign search | binary procfs arguments and literal argument comparison |
| Infer framework PID from frequent host PPID | preserve actual host PPIDs; only label a verified framework process as framework |
| First matching host only | all matching PID/PPID identities |
| No OOP match means INPROC | UNKNOWN, NOT_RUNNING, or INPROC_INFERRED with evidence |
| Print complete command line | safe executable/callsign/PID fields, no secret arguments |

The baseline handles supported executable basenames and both `-C` argument forms. Apply must add visibility/error metadata, safe directory iteration under process exit races, state normalization, remote-locality protection, and PPID rendering. Processes can share hosts without exposing every callsign; unknown/inferred remains honest in that case.

### 6. Native platform providers and provenance

Use procfs/sysfs, `uname`, `statvfs`, and structured configuration readers. Optional libsystemd sd-bus gets unit properties; do not execute `systemctl`. Native Wayland requests collect window fields below Thunder when authorized. The Realtek provider reports declared quotas, device-node presence, and supported driver metrics separately; it never reads DRM key contents.

Configuration paths come from a reviewed allowlist. JsonCpp handles JSON. Add yaml-cpp only when applying the YAML configuration task; never approximate YAML using regexes. INI/key-value readers must not source or execute files. Preserve base/override provenance; raw declared configuration is not effective configuration until merge semantics are verified.

Local Linux collectors are disabled for remote Thunder unless target identity equivalence is established. A remote snapshot still contains remote RPC data and explicit gaps; it never merges developer-host memory or processes into the target.

### 7. Backend extensions are separate owner changes

For private queues, cache state, connection summaries, catalog timestamps, DIAL/IUI/pathway state, surfaces, and player ownership, record a provider gap with owner and contract. A proposed backend `GetDiagnosticSnapshot` is a new API proposal, not an existing operation. Prefer read-only structured bounded snapshots with schema version and field availability. Use separately reviewed OpenSpec changes in the owning repository; after they ship, enable their verified adapter here.

## Risks / Trade-offs

- [Generated interface bindings unavailable] Mitigation: provisional registry entries remain disabled until SDK/target fixtures prove exact names, versions, and payload shapes.
- [Legacy debug dump mistaken for production API] Mitigation: debug-only compatibility provider is optional; operational snapshots use supported read-only contracts.
- [Unknown/partial fields reduce apparent completeness] Mitigation: explicit coverage is safer than invented data; assign backend owners and keep target parity as a release gate.
- [Procfs visibility, host sharing, PID reuse, and process-exit races] Mitigation: source timestamps, visibility metadata, bounded iteration, and inference labels; never claim an atomic snapshot.
- [Remote URL accidentally combines different machines] Mitigation: locality policy precedes collector selection.
- [Large or sensitive backend payloads] Mitigation: size/fan-out limits, allowlisted fields, recursive schema-aware redaction, escaped terminal output, and no raw credential logging.
- [Optional systemd/Wayland/GPU adapters vary by image] Mitigation: feature flags and unsupported statuses; host builds must not require target-only dependencies.

## Migration Plan

1. Review this change and approve required field coverage versus explicitly deferred backend extensions.
2. Start apply in a new request; implement the utility in small behavior-tested tasks. Keep existing script and neighboring repositories untouched.
3. Validate registry bindings using the actual Thunder SDK and sanitized target responses. Unit tests alone do not certify API coverage.
4. Cross-build against the image sysroot; package with the product's existing Yocto integration and approved runtime dependencies.
5. Run read-only device acceptance beside appsservicectl and compare the coverage manifest, not blindly the complete legacy text.
6. Install only after package and device approval. Roll back by removing/reverting this standalone package; no plugin or application state migrations are required.
7. Sync/archive OpenSpec only when implementation and agreed release gates are complete; preserve deferred backend work as separate owner changes.