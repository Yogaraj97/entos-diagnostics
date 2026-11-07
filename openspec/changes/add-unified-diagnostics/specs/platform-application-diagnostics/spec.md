## Purpose

Expose platform and application diagnostics through independent read-only providers, preserving source identity and explicit gaps in legacy dumpsys coverage.

## ADDED Requirements

### Requirement: Platform snapshot
The utility SHALL collect target-local kernel identity, uptime, memory, mount/filesystem space, input device inventory, and configured platform metadata when supported. Optional network, power, service, GPU, thermal, and pressure fields SHALL have provider and permission declarations. A missing kernel interface SHALL report unsupported; an access failure SHALL report permission-denied. Device identity SHALL NOT be inferred from the developer host or guessed from a product configuration filename.

#### Scenario: Linux memory information is available
- **WHEN** the target exposes memory and uptime kernel interfaces
- **THEN** PLATFORM reports parsed values with units and source paths.

#### Scenario: Thermal metrics do not exist
- **WHEN** the platform has no supported thermal provider
- **THEN** thermal fields are explicitly unsupported without suppressing other platform values.

### Requirement: Configuration provenance and redaction
The utility SHALL identify configured base and override files, parse supported structured formats with a parser, and show ordered provenance. Effective configuration SHALL be reported only when the backend supplies it or a verified merge policy is implemented. Secret file contents, DRM keys, credentials, identifiers designated private, and authorization data SHALL be excluded or redacted.

#### Scenario: Realtek YAML includes a DRM key path
- **WHEN** the configuration contains SVP key file entries
- **THEN** the utility does not open or copy key contents and labels the configuration as declared rather than effective GPU state.

### Requirement: Application and lifecycle snapshot
The utility SHALL retrieve installed and loaded application data through verified read-only providers. It SHALL keep application IDs, instance IDs, runtime state, lifecycle state, visibility, focus, and session fields distinct. Per-application runtime calls SHALL depend on valid discovered instance IDs and respect fan-out limits. Missing version, flags, session, or timestamps SHALL be reported individually.

#### Scenario: Application runtime and lifecycle states differ
- **WHEN** an app is suspended but its Firebolt lifecycle is initializing
- **THEN** the two states retain their original labels and are not collapsed into a single state.

### Requirement: Package and resource snapshot
The utility SHALL collect available package/version inventory, storage/quota information, preinstall status, and resource metrics using read-only operations. Package cache, mount details, active downloads, removal history, and catalog freshness SHALL appear only when a supported provider exposes them. It SHALL NOT enumerate them by performing installs, downloads, or cleanup.

#### Scenario: Package inventory exists but cache metadata is private
- **WHEN** the backend provides package names and versions but no cache entries
- **THEN** PACKAGE MANAGER is partial and its cache fields are unsupported with a backend-extension reason.

### Requirement: Window and input snapshot
The utility SHALL collect available client inventory, focus, visibility, bounds, resolution, and input inventory from verified read-only sources. It SHALL distinguish the RDK window manager service, Thunder window manager wrapper, and compositor instances. Surface IDs, player ownership, VNC connections, and display readiness SHALL NOT be inferred solely from socket existence or a running process.

#### Scenario: Wayland socket exists without a status API
- **WHEN** a compositor socket is present but surface introspection is unavailable
- **THEN** endpoint presence is reported as endpoint presence, not proof of rendering readiness or visible surfaces.

### Requirement: Gateway and startup diagnostics
The utility SHALL inventory gateway, actions, notifications, SceneSet plugin, and standalone SceneSet service health when supported. Pending launch queues, routing tables, active connections, startup decisions, forwarding state, DIAL, IUI, pathway, softcat, and removal summaries SHALL require dedicated verified read-only snapshots. Logging context getters SHALL NOT be presented as operational dumpsys snapshots.

#### Scenario: Gateway has no diagnostic snapshot API
- **WHEN** Controller exposes gateway activation but no read-only connection summary exists
- **THEN** activation is reported and connection/route coverage is unsupported without issuing resolver or action requests.

### Requirement: Backend extension contract
Every unsupported operational field SHALL identify its owning subsystem and the required read-only extension. New backend snapshots SHALL declare schema version, timestamp, bounded payload, error behavior, authorization, and field-level availability. Extensions SHALL be reviewed in the owning repositories and integrated through the utility's provider architecture without shell helpers.

#### Scenario: Add a compositor snapshot provider
- **WHEN** a reviewed compositor API exposes bounded surface and player details
- **THEN** the utility integrates those fields through the window section without changing unrelated collectors or the versioned top-level snapshot contract.