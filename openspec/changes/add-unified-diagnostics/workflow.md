# OpenSpec Workflow And Deployment Guide

## Current State

This change contains a proposal, three capability delta specs, architecture, repository/coverage analysis, and a trackable implementation checklist. Apply has produced a runnable host `entos-diagnostics` executable, RPC transport, collector engine, initial platform/input/configuration collectors, reviewed-profile application mappings and install rules. Five CTest suites pass. The larger checklist is 16/34 complete: several adapters, live bindings, Yocto packaging and target acceptance remain pending. No device deployment has been performed.

The local `openspec-propose` workflow ends at proposal review; an explicit subsequent apply request has now started implementation. Do not treat an artifact status of `done` as implementation completion: it only means the planning file exists.

## 1. Read And Review

Read [analysis.md](analysis.md) for the seven-repository evidence and legacy field map, [design.md](design.md) for C++ folder ownership, and [tasks.md](tasks.md) for implementation order.

Review these boundaries before apply:
- The utility is read-only and performs retrieval itself in C++, without shell helpers.
- All deployed Thunder plugins are inventoried; only verified read-only operations are queried. "All Thunder calls" does not mean executing setters, launch/kill/install commands, or event subscriptions indiscriminately.
- Text resembles the supplied dumpsys output but explicitly reports missing fields; there is no fabricated parity.
- Local platform/process data is never silently attached to a remote Thunder target.
- Internal state not exposed by an API needs an owning-backend change; logging contexts and debug-only dumps are not production substitutes.

Commands run from the component directory:

```bash
cd entos-diagnostics
openspec status --change add-unified-diagnostics
openspec validate add-unified-diagnostics --strict
```

If already inside the component, omit the `cd` command. The change is repo-local; no registered external OpenSpec store was selected.

## 2. Start Apply Explicitly

In a new Copilot request, use:

```text
Apply OpenSpec change add-unified-diagnostics in entos-diagnostics.
Keep all information retrieval in C++ and do not modify neighboring repositories.
Implement pending tasks with focused validation after each change.
```

The configured agent command is:

```text
/openspec-apply-change add-unified-diagnostics
```

The CLI itself provides instructions and progress; it does not implement C++:

```bash
openspec instructions apply --change add-unified-diagnostics --json
```

Implement in checklist order, mark a checkbox only after its test passes, and retain blocked target tasks as pending. Reuse the existing focused test file when practical. Use sanitized fixtures rather than real access tokens, identifiers, session secrets, or application intent payloads.

## 3. Establish Wire Contracts

For each enabled provider, record exact callsign, deployed interface version, method name, parameter schema, response schema, permissions, and a sanitized response fixture. Verify generated JSON-RPC headers in the target SDK and the plugin registration code. Do not assume a COM method is JSON-accessible or that all interface names are lowercase.

Start with Controller status, AppManager inventory, package inventory, ResourceMonitor, and window client inventory. Add lifecycle/runtime per-instance data only after their generated contracts are verified. Preserve method case: local tests establish `getLoadedApps`, `listPackages`, `getApps`, and uppercase resource methods such as `GetStats`.

Do not invoke the shell reference for collection. The implemented utility and its injectable C++ test transport should exercise these requests. If a target is not available, keep provider enablement and live verification pending rather than filling in guessed payloads.

## 4. Build And Test Locally

The current utility builds with C++17, CMake 3.16+, pkg-config, libcurl and JsonCpp. Tests cover synthetic procfs/input/configuration, real loopback HTTP, malformed RPC, authentication, deadlines, output safety, mapping, CLI selectors/exit codes and staged installation. They do not certify live target bindings or optional adapters that have not been implemented.

```bash
cmake -S . -B /tmp/entos-diagnostics-build -DBUILD_TESTING=ON
cmake --build /tmp/entos-diagnostics-build
ctest --test-dir /tmp/entos-diagnostics-build --output-on-failure
```

Install libcurl and JsonCpp development files in the host/toolchain environment. CMake, JsonCpp and yaml-cpp were installed with Homebrew on this Mac; the SDK provides libcurl. Optional YAML uses `-DENTOS_ENABLE_YAML=ON`. Native systemd/Wayland adapters are still pending and are not required for the minimal build.

Host validation must cover:
- JSON-RPC protocol/ID/error validation, malformed and oversized responses.
- Authorization-header handling, token file permissions, TLS errors and redaction.
- Unknown/inactive/inferred modes, both callsign flag forms, multiple hosts, disappearing processes and restricted procfs.
- Section selection, exit codes, deadlines, fan-out and independent partial results.
- Field coverage, configuration provenance, separate application/runtime/lifecycle/session states.
- Safe text rendering, machine-readable JSON and no subprocess information retrieval.

## 5. Cross-Compile And Package

These are future target deployment steps. The host executable and CMake install rules exist, but no cross-built device package is certified.

1. Obtain the XiOne/ES1 image SDK and use its approved CMake toolchain/sysroot. Resolve curl, JsonCpp and optional adapter libraries from that sysroot, not host Homebrew paths.
2. Confirm the product's minimum C++ runtime/toolchain and filesystem support.
3. Use the implemented CMake install rules; complete the product-reviewed Yocto recipe before device packaging.
4. Select the actual license with the component owner. No license was invented during scaffolding; the recipe cannot have a guessed license checksum.
5. Review runtime paths, configuration defaults, package dependencies and executable hardening.

For a target build after SDK/product integration:

```bash
cmake -S . -B build-target \
  -DCMAKE_TOOLCHAIN_FILE="$ENTOS_TOOLCHAIN_FILE" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_TESTING=OFF
cmake --build build-target
DESTDIR="$ENTOS_STAGING_DIR" cmake --install build-target
```

Set the toolchain and staging variables to approved local paths. Review the staging tree before packaging. Use the product's Yocto layer/package pipeline for deployment; the existing workspace does not supply that image layer, target credentials, or package feed. No direct device installation is authorized or attempted in this planning phase.

## 6. Device Acceptance

The host build already supports the following commands. Use them on a device only after the cross-built package is installed through the approved image/package mechanism:

```bash
entos-diagnostics list
entos-diagnostics plugins
entos-diagnostics dumpsys
entos-diagnostics dumpsys --section runtime --section window --format json
entos-diagnostics dumpsys --section platform --section input
```

For an authenticated endpoint, use `--token-file` with an owner-only regular file or inherited `TOKEN` containing the authorization header. Use `--url` or `THUNDER_URL` for endpoint selection. Never put real tokens in chat, screenshots, positional command arguments, source fixtures or deployment instructions.

Compare the live snapshot against appsservicectl section by section. The supplied dumpsys file is truncated and is a reference format, not a complete fixture or authoritative JSON contract. Acceptance must record observed field mappings, missing backend fields, exact deployed API versions, permissions, elapsed time, memory use, and approved privacy policy.

Do not conclude that `ready` in the application service manager equals Thunder activation, that `initializing` means a runtime is not suspended, or that a Wayland endpoint means a surface is visible. Verify those distinct data sources independently.

Required negative cases: Thunder down, authentication denied, unknown method, inactive plugin, inaccessible procfs, remote target, malformed/slow/oversized response, absent optional platform libraries, package metadata gaps and disconnected compositor. The utility must preserve unrelated successful fields and return the documented partial-result code.

## 7. Add Missing Backend Information

Create separate reviewed owner changes for the extension workstreams in [analysis.md](analysis.md). For example, run the following from the owning backend repository only after that scope is approved:

```bash
openspec new change add-read-only-operational-snapshot
```

Define read-only versioned fields, authorization, payload limits, timestamp and error behavior before implementing the backend. After that backend API ships, add a sanitized fixture and verified provider entry in this component. Never enable a speculative `GetDiagnosticSnapshot` method before it actually exists.

## 8. Validate, Sync And Archive

After implementation, target acceptance and approved coverage are complete:

```bash
openspec validate add-unified-diagnostics --strict
openspec status --change add-unified-diagnostics
```

Then use the configured archive workflow:

```text
/openspec-archive-change add-unified-diagnostics
```

Alternatively inspect `openspec archive --help` and run the CLI archive flow after review. Main specs remain empty during proposal planning; archive/sync installs the accepted delta into the source-of-truth specs. Do not archive with unchecked release tasks or claim OpenSpec validation proves runtime correctness.

Rollback is package removal/reversion through the same approved image/package mechanism. Because this utility is standalone and read-only, no app, plugin, or configuration-state migration is required.

## Verification Recorded During Preparation

- CMake configure/build and CTest: passed, five suites covering core/fixtures, help, list, real HTTP transport and CLI exit behavior.
- YAML-enabled host build: passed, including nested Realtek-style configuration with declared-state semantics.
- Minimal YAML-disabled host build: passed, all five suites, without linking yaml-cpp.
- Staged CMake installation: passed under `/tmp/entos-diagnostics-stage`; executable and profile/schema files only.
- Strict OpenSpec validation: passed during planning; apply close-out revalidates the current artifacts.
- Live Thunder, complete optional adapters, cross-build, Yocto packaging and Realtek acceptance/deployment: not performed and remain pending.

## Host Apply Commands Recorded

```bash
cmake -S . -B build -DENTOS_ENABLE_YAML=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
DESTDIR=/tmp/entos-diagnostics-stage cmake --install build
```

Provisional profiles remain disabled. Enable only a product-reviewed verified profile with exact response field mappings; unmapped raw application payloads are deliberately omitted from output. Continue unchecked tasks rather than archiving this partially applied change.