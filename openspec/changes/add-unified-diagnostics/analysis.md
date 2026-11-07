# Repository Evidence And Diagnostic Coverage

This is a targeted architecture/API analysis of all seven requested folders, not an exhaustive review of every source file or proof of deployed wire contracts. The requested `rdk-windowmanager` folder corresponds to the actual `rdk-window-manager` checkout. Findings distinguish local source evidence from target validation still required. This document describes proposal-time coverage; the component README and checklist record current apply progress.

## Repository Responsibilities

| Repository | Observed responsibility | Diagnostic integration |
| --- | --- | --- |
| entos-appmanagers | Application-service layer, configuration, catalog/startup management, Thunder and DBus variants, package/runtime dumpsys services | Reuse diagnostic semantics, not internal library linkage; public getters where verified; debug Dump compatibility only after transport verification |
| entservices-appgateway | AppGateway resolver/responder, actions, notifications, transport and context management | Inventory plugin activation; dedicated read-only connection/routing snapshots needed for full gateway diagnostics |
| entservices-appmanagers | AppManager, lifecycle/runtime/package/preinstall/storage managers, resource monitor, window wrapper, telemetry and victim selection | Main app/package/lifecycle/resource getters; do not invoke launch/kill/install/evict/record methods |
| entservices-sceneset | Thunder SceneSet plus broad platform/service plugins, including alternative copies of several manager implementations | Select the deployed implementation/version, not both copies; verified platform read APIs and Controller inventory; dedicated private-state snapshots where necessary |
| rdk-app-manager-configs-soc-realtek | XiOne/ES1 app/container GPU configuration and service overrides | Configuration provenance, declared RAM/GPU/SVP settings, device nodes; no secret key reads or claims of effective driver state |
| sceneset | Standalone systemd application launcher, first-boot preinstall, update/relaunch monitoring | Native service health and declared reference-app configuration; startup/update decision snapshot needs an owner API |
| rdk-window-manager | Native Wayland/Westeros/Essos compositor, input, Firebolt protocols, optional VNC | Thunder wrapper plus optional native read-only Firebolt WM requests; deep surface/player/VNC details need additional contracts |

## Concrete Evidence

- [DumpSys.cpp](../../../../entos-appmanagers/rdkappmanagers-thunder/ASAppManager/apps/dumpsys/source/DumpSys.cpp) registers named services, matches patterns, and isolates service exceptions. Its fd-based internal API is not itself a public JSON-RPC endpoint.
- [CommandHandler.cpp](../../../../entos-appmanagers/rdkappmanagers-thunder/ASAppManager/CommandHandler.cpp#L722) implements debug-only `Dump` with `serviceName` and optional `args`, returning `success` and `output`. `ListServices` is also debug-only. The utility must verify the externally reachable command transport before using either.
- [ASAppManagerPlugin.cpp](../../../../entos-appmanagers/rdkappmanagers-thunder/ASAppManager/ASAppManagerPlugin.cpp#L75) registers application service request/config/listener/system-info/settings/test-preference/diagnostics interfaces. Diagnostic contexts are logging configuration, not full dumpsys snapshots.
- [AppManagerImplementation.h](../../../../entservices-appmanagers/AppManager/AppManagerImplementation.h#L207) exposes loaded/installed apps and read-only properties/metadata/limits. [test_AppManager.cpp](../../../../entservices-appmanagers/Tests/L1Tests/tests/test_AppManager.cpp#L626) verifies camel-case JSON-RPC names such as `getInstalledApps` and `getLoadedApps`.
- [LifecycleManagerImplementation.cpp](../../../../entservices-appmanagers/LifecycleManager/LifecycleManagerImplementation.cpp#L233) builds loaded-app JSON with `appInstanceID`, `appId`, numeric lifecycle/target states, last-change time, active session, recent intent, and optional `runtimeStats`. Preserve the exact `appInstanceID` capitalization and redact sensitive intent content.
- [RuntimeManagerImplementation.h](../../../../entservices-appmanagers/RuntimeManager/RuntimeManagerImplementation.h#L187) exposes `GetInfo(appInstanceId, info)`. It needs valid instance IDs and verified wire binding; Run/Suspend/Kill/Annotate/Mount are not diagnostic collection calls.
- [PackageManager.cpp](../../../../entservices-appmanagers/PackageManager/PackageManager.cpp#L85) registers downloader, installer, package configuration, and package-status bindings. The package-handler binding is commented out; COM availability does not prove JSON availability.
- [PackageManager.conf.in](../../../../entservices-appmanagers/PackageManager/PackageManager.conf.in) derives the callsign from `PLUGIN_NAME`; do not assume all builds call it `org.rdk.PackageManager`. The standalone SceneSet documents `org.rdk.AppPackageManager`.
- [test_PackageManager.cpp](../../../../entservices-appmanagers/Tests/L1Tests/tests/test_PackageManager.cpp#L365) verifies `listPackages`. Inventory is not proof of cache entries, mounted packages, or pending installer queues.
- [test_ResourceMonitor.cpp](../../../../entservices-appmanagers/Tests/L1Tests/tests/test_ResourceMonitor.cpp#L213) verifies uppercase `GetMemInfo`, `GetPsiMetrics`, `GetSwapUsed`, `GetFlashSpace`, and `GetStats`; lowercase guessing would fail.
- [test_RDKWindowManager.cpp](../../../../entservices-appmanagers/Tests/L1Tests/tests/test_RDKWindowManager.cpp#L278) verifies `getApps`; other wrapper getters require exact binding/parameter evidence before enablement.
- [AppGatewayImplementation.h](../../../../entservices-appgateway/AppGateway/AppGatewayImplementation.h#L54) and [AppGatewayResponderImplementation.h](../../../../entservices-appgateway/AppGateway/AppGatewayResponderImplementation.h#L58) expose resolving/responding/context operations, not a bounded diagnostic summary. Actions and notifications must not be triggered to obtain health.
- [SceneSet.cpp](../../../../entservices-sceneset/SceneSet/SceneSet.cpp#L54) queries AppManager, registers lifecycle notifications, and starts the reference app. This Thunder plugin is distinct from the standalone launcher.
- [README.md](../../../../sceneset/README.md) and [configuration.md](../../../../sceneset/docs/configuration.md) describe the standalone launcher's systemd/COM-RPC dependencies, configuration layers, preinstall, firmware/update behavior, and reference-app overrides.
- [rdkappmanagers-config.yaml](../../../../rdk-app-manager-configs-soc-realtek/config/xione/rdkappmanagers-config.yaml) declares memory/GPU quotas, mounts, SVP entries, and GPU overrides. Its own comment says `gpuConfig` is not currently parsed by AIConfiguration/RuntimeManager; declared YAML cannot be labelled effective configuration.
- [realtek.conf](../../../../rdk-app-manager-configs-soc-realtek/config/es1/realtek.conf) is a service override, not a status API. Diagnostics must parse configuration safely and must not execute its shell-style directives.
- [firebolt_wm.xml](../../../../rdk-window-manager/extensions/firebolt_wm/protocol/firebolt_wm.xml) defines native `get_clients`, `get_properties`, `get_focused_client`, and `get_owner` requests and response events. Mutating requests are excluded from the diagnostic adapter.

## Read-Only Binding Inventory

`Name verified` below means local tests/documentation establish spelling, not that the deployed interface version, response schema, permissions, or availability have been certified. All entries require target fixtures before release.

| Provider | Candidate operation | Evidence level | Required verification |
| --- | --- | --- | --- |
| Controller | `Controller.1.status` | Supplied script intent | Actual result envelope/shape and deployed version |
| AppManager | `getInstalledApps`, `getLoadedApps` | Name verified by JSON-RPC tests | Callsign/version, iterator versus embedded-JSON shape |
| AppManager | `getAppProperty`, `getAppMetadata`, `getMaxRunningApps`, `getMaxHibernatedApps`, `getMaxHibernatedFlashUsage`, `getMaxInactiveRamUsage` | Name verified by JSON-RPC tests | Required parameters, field schema, privacy |
| LifecycleManager | COM `GetLoadedApps(verbose, apps)` | Implementation and COM contract verified | Generated JSON binding and enum definitions; provisional |
| RuntimeManager | COM `GetInfo(appInstanceId, info)` | Header verified | Generated JSON binding and payload; provisional |
| AppPackageManager/PackageManager | `listPackages` | Name verified by JSON-RPC test | Configured callsign/version and iterator payload |
| Package provider | COM `GetStorageInformation`, package state/config getters | Interface-level evidence | Exact exposed binding/parameters; provisional |
| PreinstallManager | COM `GetPreinstallState` | Interface-level evidence | Generated JSON name/version and enum; provisional |
| ResourceMonitor | `GetMemInfo`, `GetPsiMetrics`, `GetSwapUsed`, `GetFlashSpace`, `GetStats` | Name verified by JSON-RPC tests | Metrics/units, `GetPsiMetrics` parameters, version |
| RDKWindowManager | `getApps` | Name verified by JSON-RPC test | Deployed version and client iterator shape |
| RDKWindowManager | COM visibility/render/last-key getters | Header-level evidence | Exact bindings and data/permission policy; provisional |
| RDKShell | Registered read-only getter constants in implementation | Registration evidence | Resolve actual string constants and target profile before enablement |
| ASAppManager | COM `GetSystemInfo`, `GetSystemSetting`, `GetDiagnosticContexts` | Interface registered | Generated JSON contract; contexts are logging only |
| ASAppManager debug command transport | `Dump`, `ListServices` | Internal command implementation | External transport, debug feature flags, access policy; off by default |
| Gateway/actions/notifications | Controller inventory only | Plugin/service sources | New structured operational snapshot for routes/connections |
| Other platform/service plugins | Controller inventory plus reviewed per-plugin getters | Mixed interface sources | Build/image-specific registry review, no blind enumeration |
| Firebolt WM | Native read-only client/property/focus/owner requests | Protocol verified | Socket access, event completeness, timeout, target identity |

The broad entservices-sceneset checkout also contains analytics, settings, stores, telemetry, monitor, resource management, browser/container, and USB services. Controller discovery covers their presence. A diagnostic registry must classify every deployed callsign, but must not blindly call every operation: private store reads, setters, emitters, and actions are not universal diagnostics. Alternative manager copies require build-profile selection.

## Legacy Dumpsys Coverage And Owners

This matrix maps every supplied section to retrieval opportunities and known gaps. It is a planning map, not certification of complete implemented coverage; initial collectors now exist and the checklist identifies remaining work.

| Stable section ID / Heading | Retrieval provision | Fields requiring additional backend evidence or extension |
| --- | --- | --- |
| config / CONFIG | JSON/YAML/INI provenance provider; AS config getter if verified | Effective merge policy, active override order and actual consumed GPU configuration |
| dial / DIAL | Controller inventory, optional dedicated DIAL/XCast read snapshot | Enabled/started/powered, protocol/device description, wakeup and live URLs |
| iui / DYNAMIC IUI MANAGER | AS settings/config and dedicated IUI snapshot | Effective RFC and latest selector decision |
| forwarded-launch / FORWARDED LAUNCH MANAGER | AS manager owner snapshot | Pending request and effective enablement |
| input / INPUT HANDLER | Native Linux input/sysfs inventory; configured keymap parser | Active loaded mapping, semantic remapping, live intercept/listener summary |
| package / PACKAGE MANAGER | Package list, reviewed quota/state getters, filesystem space | Installer queue/thread details, cache LRU/size/in-use and package mount attribution |
| pathway / PATHWAY MANAGER | Declared settings plus owner snapshot | Installer activity, effective RFC, last successful fetch and redacted device context |
| platform / PLATFORM | uname/procfs/statvfs; reviewed system-info/config APIs | Firmware/platform/partner/activation/locale values and privacy-approved device identity |
| power / POWER STATE MANAGER | Reviewed product power plugin/provider | Current/previous states and transition reason; no state inference from app activity |
| removal / REMOVE UNUSED APPS MANAGER | Catalog/removal owner snapshot | Last run timestamp and completed/skipped/failed counts |
| runtime / RUNTIME MANAGER | Loaded/installed apps, lifecycle verbose data, per-instance runtime info | Separate GUI/daemon membership, runtime flags, AS/Firebolt/FTL session mapping where not exposed |
| softcat / SOFTCAT | Catalog owner snapshot | Effective URL/baseline, refresh schedule/status, per-package state at refresh |
| startup / STARTUP APP LAUNCHER | SceneSet unit properties, declared ref app, AS startup owner snapshot | Pending prelaunch queue, next deadline, recently used ordering and effective network/policy state |
| system-app / SYSTEM APP MANAGER | Controller and application inventory plus system-app owner snapshot | System service launch timestamps and app/service association |
| thunder / THUNDER SERVICE MANAGER | Controller status + process evidence; optional connection health | Application-manager readiness is distinct from Thunder activation; security token is always redacted |
| window / WINDOW MANAGER | Wrapper getters, native Firebolt WM, compositor process/endpoint presence | Player/subtitle/watermark surfaces, exact ownership semantics, client PIDs, VNC connections, splash/blanking/error OSD and frame rate |

## Backend Extension Workstreams

1. AS application-service owners: production read-only snapshots for startup/IUI/forwarding/pathway/DIAL/catalog/removal and GUI/daemon/session details. Do not expose debug mutating commands to obtain them.
2. Gateway owners: bounded counts/status for routing and connection health without credentials or subscriber payloads.
3. Package/download owners: installer requests, cache attribution, mounts, download progress/queue and freshness; avoid private file scraping.
4. Window/compositor owners: surface/player/subtitle/watermark/VNC details not covered by current Firebolt WM getters.
5. Platform owners: verified firmware, power, locale, thermal/GPU and systemd contracts plus privacy-approved identity.

Suggested new backend snapshot contract: read-only, schema-versioned, timestamped, bounded, authenticated, explicitly non-atomic, with per-field availability and sanitized errors. The name `GetDiagnosticSnapshot` is a proposal only. Each owner workstream needs its own reviewed OpenSpec change before this utility enables a binding.

## Script Defects Addressed In The Plan

The attached script assumes top-level `.plugins` despite issuing JSON-RPC, expands authorization as a split shell string, has a fragile jq-less JSON fallback, interprets callsigns as regexes, reports INPROC on absence of evidence, selects only one host, and correlates local processes with any configured remote URL. The C++ design replaces each of those behaviors rather than translating those errors literally. The script remains untouched as an input reference.