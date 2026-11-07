## Purpose

Provide one read-only diagnostic utility with understandable text output and a machine-readable snapshot across AI2.0 services and their underlying platform.

## ADDED Requirements

### Requirement: Unified command interface
The utility SHALL expose `dumpsys`, `plugins`, `list`, and `--help`. `dumpsys` SHALL accept repeatable `--section` selectors and `--format text|json`. Unknown commands, sections, formats, and invalid options SHALL fail before collection with exit code 2.

#### Scenario: Select application and window sections
- **WHEN** an operator requests `dumpsys --section runtime --section window --format json`
- **THEN** the output contains only the selected sections and snapshot metadata, without unrelated collection calls.

#### Scenario: Invalid selection
- **WHEN** an operator selects an unknown section
- **THEN** the utility reports valid section names and exits 2 without network requests.

### Requirement: Common snapshot contract
The utility SHALL report schema version, collection start time, elapsed time, target identity, collector identity, section status, source provenance, data, and field-level availability. Status SHALL be one of `ok`, `partial`, `unsupported`, `permission-denied`, `timeout`, `unavailable`, or `error`. Independently observed fields SHALL retain their own source and collection time. Required field coverage SHALL determine section completeness.

#### Scenario: One section fails
- **WHEN** package collection times out but runtime collection succeeds
- **THEN** the utility preserves the runtime data and reports the package timeout instead of aborting the whole snapshot.

### Requirement: Dumpsys-style presentation
Text output SHALL use named sections and aligned tables for plugins, packages, applications, and window clients. Missing values SHALL display `--` with a reason. The initial coverage manifest SHALL include CONFIG, DIAL, DYNAMIC IUI MANAGER, FORWARDED LAUNCH MANAGER, INPUT HANDLER, PACKAGE MANAGER, PATHWAY MANAGER, PLATFORM, POWER STATE MANAGER, REMOVE UNUSED APPS MANAGER, RUNTIME MANAGER, SOFTCAT, STARTUP APP LAUNCHER, SYSTEM APP MANAGER, THUNDER SERVICE MANAGER, and WINDOW MANAGER. Exact legacy field parity SHALL NOT be claimed until those fields are verified on target.

#### Scenario: Backend does not expose pathway state
- **WHEN** no supported provider can retrieve the last pathway fetch
- **THEN** PATHWAY MANAGER remains in a full dump with `unsupported` field coverage rather than a fabricated timestamp.

### Requirement: Stable output and safe rendering
JSON output SHALL remain parseable and use a versioned schema. Text output SHALL escape control characters, redact secrets, and preserve complete values without terminal-control injection. Sorting SHALL be deterministic; provider-defined chronological application ordering SHALL be preserved where it has meaning.

#### Scenario: Malicious application name
- **WHEN** a backend returns a name containing a terminal escape sequence
- **THEN** text rendering escapes the sequence and JSON rendering contains a valid escaped string.

### Requirement: Bounded collection and exit semantics
The utility SHALL support connect, per-request, and total snapshot deadlines, response size limits, and bounded fan-out. It SHALL exit 0 only when every selected required section is complete, 1 for a partial diagnostic result, and 2 for invalid input or invalid configuration. Deadline exhaustion SHALL produce a valid partial snapshot with reasons for uncollected sections.

#### Scenario: Global deadline expires
- **WHEN** slow per-application calls exceed the snapshot deadline
- **THEN** the utility stops scheduling calls, cancels outstanding work within the deadline, retains completed data, and exits 1.

### Requirement: Extensible single-utility retrieval
The utility SHALL support independently registered Thunder, platform, and application providers, without a shell command or subprocess for information retrieval. A provider extension SHALL declare its section, target locality, data fields, permissions, read-only operations, and required coverage.

#### Scenario: Platform provider is disabled
- **WHEN** optional GPU or systemd support is not compiled into the utility
- **THEN** its fields report `unsupported` while other selected sections continue to work.

### Requirement: Deployable diagnostic package
The implementation SHALL provide native and cross-build instructions, install the executable and versioned configuration through the build system, and document runtime dependencies, minimum privileges, device acceptance, and rollback. Production acceptance SHALL require target tests separate from host unit tests.

#### Scenario: Host tests pass without a device
- **WHEN** host unit tests complete but no Realtek device has been tested
- **THEN** implementation status explicitly leaves target deployment and live API acceptance incomplete.