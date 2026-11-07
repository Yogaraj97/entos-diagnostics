## Purpose

Collect trustworthy Thunder plugin status and read-only responses, including execution-mode evidence, without mutating services or depending on shell utilities.

## ADDED Requirements

### Requirement: Validated JSON-RPC transport
Requests SHALL use JSON-RPC 2.0 with unique IDs, exact method spelling, correctly encoded parameters, bounded HTTP POST transport, and certificate verification for HTTPS. Responses SHALL validate JSON syntax, protocol version, matching ID, and exactly one of result or error. HTTP failures, authentication failures, JSON-RPC errors, and invalid envelopes SHALL be distinct. Embedded JSON strings SHALL be decoded only for fields whose verified contract declares embedded JSON.

#### Scenario: Controller returns a wrapped array
- **WHEN** Controller returns a valid matching envelope with a plugin array in `result`
- **THEN** plugin discovery reads that array rather than expecting a top-level `plugins` member.

#### Scenario: Wrong request ID
- **WHEN** a successful HTTP response has a nonmatching JSON-RPC ID
- **THEN** the call reports a protocol error and no result is associated with that request.

#### Scenario: Error response
- **WHEN** a plugin returns a JSON-RPC error on HTTP 200
- **THEN** the utility reports the error code as a failed call rather than treating HTTP success as data success.

### Requirement: Safe authentication
The utility SHALL accept credentials through a protected token file or inherited environment, never a positional command-line token. It SHALL construct a single authorization header, reject header-control characters, and redact credentials from all text, JSON, errors, and diagnostic files. It SHALL NOT automatically obtain tokens, elevate privileges, disable TLS verification, or change plugin state.

#### Scenario: Bearer token contains spaces
- **WHEN** the inherited token value is a valid bearer header
- **THEN** it is sent as one authorization header and never split into transport arguments or printed.

### Requirement: Discover all plugins safely
The utility SHALL inventory all unique callsigns returned by the verified Controller status API, retaining activation state and separately labelled lifecycle/status/phase values when available. Unknown plugin types SHALL appear in `plugins` output. Missing or malformed status data SHALL be explicit. An empty valid inventory SHALL be distinguished from an invalid response.

#### Scenario: Duplicate callsign
- **WHEN** Controller returns the same callsign twice
- **THEN** one row is emitted with conflicting evidence identified rather than silently choosing an arbitrary state.

### Requirement: Read-only API coverage
The utility SHALL maintain a registry of exact callsigns, interface versions, method names, parameter contracts, response contracts, evidence sources, and diagnostic field mappings. Only reviewed read-only operations SHALL be invoked. Unverified bindings SHALL be disabled by default. All discovered plugins SHALL be classified as mapped or unmapped; advertised methods SHALL NOT be automatically executed. Unknown-method errors SHALL permit only explicitly configured read-only fallback candidates.

#### Scenario: Plugin advertises a launch operation
- **WHEN** a plugin is discovered with launch, install, kill, write, or purge operations
- **THEN** none of those operations is invoked during diagnostics.

#### Scenario: Interface exists but JSON binding is unverified
- **WHEN** an implementation header declares a getter without a verified JSON-RPC contract
- **THEN** the registry marks it provisional and the default snapshot reports unavailable coverage without guessing method capitalization.

### Requirement: Native process evidence
On the target, the utility SHALL inspect Linux process arguments and parent PIDs natively. It SHALL recognize supported host executable names and exact `-C callsign` and `-Ccallsign` arguments, report every matching host PID and PPID, and match callsigns literally. Raw command lines SHALL NOT expose tokens or sensitive arguments.

#### Scenario: Callsign includes regex metacharacters
- **WHEN** `org.rdk.System` and `orgXrdkXSystem` exist
- **THEN** a host argument for one callsign never matches the other.

#### Scenario: Multiple matching hosts
- **WHEN** a process snapshot contains two matching host processes
- **THEN** both process identities are reported instead of arbitrarily displaying only the first.

### Requirement: Evidence-based mode classification
Mode SHALL distinguish `OOP`, `INPROC_INFERRED`, `NOT_RUNNING`, and `UNKNOWN`. A matching host is evidence for OOP. Framework presence and an active plugin without a visible host SHALL be reported only as an inference, with snapshot visibility limitations. The absence of an OOP match SHALL NOT prove INPROC. Configuration-based declared mode SHALL be separate from observed mode.

#### Scenario: Inactive plugin with no host
- **WHEN** a plugin is deactivated and no matching host exists
- **THEN** its observed mode is NOT_RUNNING, not INPROC.

#### Scenario: Host visibility is restricted
- **WHEN** procfs is absent or process arguments cannot be read
- **THEN** mode is UNKNOWN or explicitly qualified inference, with a visibility reason.

### Requirement: Target-local process correlation
Local procfs evidence SHALL NOT be associated with a remote Thunder target unless explicit target identity equivalence is established. Remote queries without a target-side process provider SHALL show mode UNKNOWN and identify process evidence as unavailable for that target.

#### Scenario: Remote Thunder URL on a developer Mac
- **WHEN** the utility queries a nonlocal target
- **THEN** the developer machine's process list never determines the target plugin's execution mode.