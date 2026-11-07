## 1. Baseline And API Contracts

- [x] 1.1 Add C++17 CMake library/test targets and native procfs process/mode baseline; verify direct Clang compilation.
- [x] 1.2 Test split/joined callsign arguments, literal matching, inactive mode, and missing-process UNKNOWN; verify CMake build and CTest.
- [x] 1.3 Define versioned provider and coverage schemas for every section in analysis.md; reject invalid bindings, mutating operations, and unknown config versions in tests.
- [ ] 1.4 Verify Controller and initial app/package/resource/window bindings against generated SDK interfaces and sanitized target responses; leave unverified candidates disabled.

## 2. Snapshot Core And CLI

- [x] 2.1 Implement common snapshot, field availability, collector context, and source/timestamp models; test status aggregation independently of rendering.
- [ ] 2.2 Implement dependency-aware collection, cancellation, total budget, fan-out, and per-provider failure isolation; test partial results and deadline exhaustion.
- [x] 2.3 Implement dumpsys/plugins/list/help, selectors, format and deadline options, configuration precedence, and exit codes; test invalid input before side effects.
- [x] 2.4 Implement JSON/text renderers with section headings and plugin/app/package/client tables; test deterministic ordering, empty values, terminal-control escaping, and JSON round trips.
- [x] 2.5 Implement schema-aware redaction for credentials, URL secrets, configuration, intent payloads and process data; test that injected secrets never appear in any output or error.

## 3. Thunder Collection

- [x] 3.1 Add installed libcurl/JsonCpp dependencies and RAII injectable HTTP transport; test connect/request deadlines, cancellation, response limits, and HTTP/TLS/authentication errors.
- [x] 3.2 Implement unique request IDs, JSON-RPC envelope validation and verified embedded-JSON adapters; test wrong IDs, malformed JSON, result/error conflicts, null results and RPC errors.
- [ ] 3.3 Implement Controller plugin discovery, all-callsign inventory, duplicate/conflicting-state handling and mapped/unmapped coverage; test actual sanitized result-shape fixtures.
- [x] 3.4 Implement safe token environment/file handling, single-header construction, CR/LF rejection and file permission checks; test without printing real credentials.
- [x] 3.5 Implement reviewed per-profile callsign/version selection and unknown-method-only fallback; prove no advertised mutating method is called.
- [x] 3.6 Harden procfs iteration for disappearing processes and permissions, add visibility/PPID evidence and state normalization, enforce target locality and redact process details; test fake procfs fixtures and remote target exclusion.

## 4. Platform Providers

- [ ] 4.1 Implement target-local uname/uptime/memory/filesystem/mount providers with units and source identity; test fake kernel roots and unsupported/permission cases.
- [ ] 4.2 Implement JSON configuration provenance, ordered override reporting and reviewed merge policies; test declared versus effective values and secret exclusion.
- [x] 4.3 Add optional yaml-cpp structured Realtek YAML configuration support; test nested mounts/GPU/SVP data without opening any key file.
- [ ] 4.4 Implement Linux input inventory and supported keymap parsing; distinguish configured mappings from confirmed active mappings in tests.
- [ ] 4.5 Implement optional native systemd unit and approved power/network adapters; test disabled builds, denied access and backend unavailability without subprocesses.
- [ ] 4.6 Implement optional Realtek device/thermal/pressure/GPU metrics behind declared provider contracts; test absent nodes and declared quotas versus observed usage.

## 5. Application And Underlying Service Providers

- [ ] 5.1 Implement installed/loaded app normalization and verified per-app metadata, preserving chronological order where defined; test separate app/instance/session identities.
- [ ] 5.2 Implement verified lifecycle verbose and bounded per-instance runtime adapters; test embedded JSON, exact appInstanceID mapping, numeric enums, distinct runtime/lifecycle states and missing IDs.
- [ ] 5.3 Implement verified package inventory, quota/storage, preinstall and resource adapters; test absent cache/queue/mount data as explicit field gaps.
- [ ] 5.4 Implement verified window wrapper client/focus/visibility/bounds adapters; test per-client failures and missing surface/player fields.
- [ ] 5.5 Implement optional native Firebolt WM client/property/focus/owner adapter with event completion and deadlines; test disconnects, partial events and read-only requests.
- [ ] 5.6 Implement gateway/startup/platform-service health collectors and full legacy-section gap reporting; test no resolver/action/event requests and no use of logging contexts as operational snapshots.
- [x] 5.7 Define component-side adapter contracts and owner references for missing DIAL/IUI/pathway/forwarding/catalog/removal/cache/player/VNC snapshots; test disabled extension entries and unsupported field reporting. Do not modify backend repositories in this task.

## 6. Release And Acceptance

- [x] 6.1 Add install rules for the executable and reviewed versioned configs; verify an unprivileged staged install contains no token material.
- [ ] 6.2 Add a product-reviewed Yocto recipe template with actual license decision, pinned source revision and dependency mapping; cross-build against the selected target sysroot.
- [x] 6.3 Run all host tests including fixture-backed RPC, renderer, security, deadline, collector and filesystem checks; record commands and results in workflow.md.
- [ ] 6.4 Run read-only XiOne/ES1 acceptance against live Thunder, restricted procfs, remote queries and unavailable plugins; compare each required field to appsservicectl and record approved/deferred coverage.
- [ ] 6.5 Verify package installation/rollback, memory and elapsed-time budget on target, then publish supported callsign/version/provider profiles and deployment evidence.
- [ ] 6.6 Strict-validate OpenSpec and sync/archive only after implemented tasks and agreed release gates pass; keep unshipped backend extensions in separately reviewed owner changes.

## Apply Status

Host-side implementation is runnable; 16 of 34 tasks are complete. Native platform/input/configuration, plugin discovery and generic application mapping also have partial implementations, but their larger tasks remain unchecked where required behavior or target evidence is missing.

Remaining work includes complete cancellation/fan-out coverage, effective configuration/keymap policies, optional native systemd/Wayland/Realtek metrics, and full application-specific normalization/per-instance adapters. Target-dependent work requires the actual generated Thunder interfaces, sanitized device responses, approved provider profiles, product license/Yocto integration, and XiOne/ES1 access. This change is not ready to archive.