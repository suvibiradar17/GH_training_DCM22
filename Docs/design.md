# Software Design Description: Diagnostic DID Read MVP

**Status:** Draft for training implementation  
**Requirement:** DCM-MVP-001  
**Source:** [AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md](AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md)

## 1. Purpose and scope

This document describes a standalone C99 module that simulates the request-processing logic for one simplified UDS ReadDataByIdentifier service (`0x22`). The module handles one DID per request and responds from a fixed, local configuration table.

The implementation runs on a local PC. It is a training MVP, not a complete ISO 14229 implementation or production AUTOSAR DCM component. CAN, ISO-TP, AUTOSAR BSW integration, diagnostic sessions, security, and ECU data callbacks are outside this design.

## 2. Design goals and constraints

- Provide deterministic request processing with no dynamic memory allocation.
- Use caller-provided request and response buffers.
- Validate pointers, request length, and response capacity before accessing or writing data.
- Keep the public interface small and separate request processing from DID configuration.
- Preserve the response bytes specified by DCM-MVP-001.
- Avoid implying complete UDS or AUTOSAR conformance.

No formal design pattern has been selected. The design uses a single request-processing function and a fixed DID lookup table; no framework or layered AUTOSAR architecture is introduced.

## 3. Proposed module structure

```text
include/dcm_mvp.h   Public status type, constants, and API declaration
src/dcm_mvp.c       Validation, DID lookup, and response construction
tests/test_dcm_mvp.c Unit tests for requirement behavior and API errors
```

This is a proposed source layout. The current repository has not yet created these implementation directories or files.

### 3.1 Responsibilities

| Element | Responsibility |
|---|---|
| Public header | Declare the API, status values, service/response identifiers, and fixed protocol lengths. |
| Request processor | Validate arguments and request format, decode the DID, find its configuration, check output capacity, and build the response. |
| DID configuration | Store the supported DID identifiers and their fixed-length synthetic test payloads. |
| Unit tests | Verify response bytes, returned status and length, malformed requests, and buffer boundaries. |

## 4. Public interface

The proposed interface follows the training brief:

```c
typedef enum
{
    DCM_MVP_OK = 0,
    DCM_MVP_INVALID_PARAM,
    DCM_MVP_BUFFER_TOO_SMALL
} DcmMvp_StatusType;

DcmMvp_StatusType DcmMvp_ProcessRequest(
    const uint8_t *request,
    uint16_t requestLength,
    uint8_t *response,
    uint16_t responseCapacity,
    uint16_t *responseLength);
```

`request` is encoded as `[SID, DID high byte, DID low byte]`. The valid request length is exactly three bytes. DID decoding uses network byte order (high byte first).

### 4.1 API behavior

- A successfully constructed positive or diagnostic negative response returns `DCM_MVP_OK` and sets `*responseLength` to the number of response bytes.
- Null required pointers return `DCM_MVP_INVALID_PARAM`. If `responseLength` is non-null, set it to zero before returning an error.
- A request with a length other than three bytes, or a SID other than `0x22`, is a valid API call that produces the specified diagnostic negative response, provided the response buffer has enough capacity.
- Insufficient response capacity returns `DCM_MVP_BUFFER_TOO_SMALL` and sets `*responseLength` to zero when that pointer is valid.
- On API errors, do not write response bytes. In particular, check the full required response capacity before writing any byte.

## 5. Protocol constants and configured data

Use named constants in the implementation rather than unexplained numeric literals.

| Meaning | Value / length |
|---|---:|
| Request SID (ReadDataByIdentifier) | `0x22` |
| Positive response SID | `0x62` |
| Negative response marker | `0x7F` |
| Request Out Of Range NRC | `0x31` |
| Incorrect Message Length Or Invalid Format NRC | `0x13` |
| Request length | 3 bytes |
| DID field length | 2 bytes |
| Negative response length | 3 bytes |
| `0xF190` data length | 17 bytes |
| `0xF187` data length | 10 bytes |

The configured DIDs are `0xF190` (VIN test data) and `0xF187` (spare-part-number test data). Their exact payload contents are not specified by the brief. Select clearly fictional values for local tests/configuration; never use real vehicle-linked identifiers or customer data. Store payload lengths explicitly or derive them with a compile-time-safe method appropriate to the chosen C99 implementation.

The positive response layout is `[0x62, DID high byte, DID low byte, data...]`. Therefore the positive response lengths are 20 bytes for `0xF190` and 13 bytes for `0xF187`.

## 6. Request processing design

Process each request in this order:

1. If `responseLength` is non-null, initialize it to zero. Validate all required pointers; return `DCM_MVP_INVALID_PARAM` if any are null.
2. Require `requestLength == 3`. If not, prepare negative response `[0x7F, 0x22, 0x13]`.
3. For a three-byte request, require `request[0] == 0x22`. If not, prepare `[0x7F, 0x22, 0x13]`.
4. Decode the DID from `request[1]` and `request[2]` in high-byte-first order.
5. Look up the DID in the fixed configuration. If it is not present, prepare `[0x7F, 0x22, 0x31]`.
6. Determine the complete response length and compare it with `responseCapacity`. If capacity is insufficient, return `DCM_MVP_BUFFER_TOO_SMALL` without writing response bytes.
7. Write the complete response and set `*responseLength` only after the response has been built.

All response paths must establish the required response length before the first write. The output buffer must never be partially populated on a capacity error.

### 6.1 Processing flow

```text
Start
  -> Initialize output length when possible; validate pointers
  -> Invalid pointer? -- yes -> INVALID_PARAM
  -> Request length is 3? -- no -> NRC 0x13
  -> SID is 0x22? -- no -> NRC 0x13
  -> Decode DID and look up configuration
  -> DID supported? -- no -> NRC 0x31
  -> Calculate complete response length
  -> Capacity sufficient? -- no -> BUFFER_TOO_SMALL
  -> Construct positive or negative response
  -> Set output length; return OK
```

## 7. Response behavior

| Input condition | Response bytes | API status |
|---|---|---|
| SID `0x22`, DID `0xF190` | `62 F1 90` followed by 17 configured bytes | `DCM_MVP_OK` |
| SID `0x22`, DID `0xF187` | `62 F1 87` followed by 10 configured bytes | `DCM_MVP_OK` |
| SID `0x22`, unsupported DID | `7F 22 31` | `DCM_MVP_OK` |
| Request length other than 3 | `7F 22 13` | `DCM_MVP_OK` |
| Request SID other than `0x22` | `7F 22 13` | `DCM_MVP_OK` |
| Required pointer is null | No response is produced | `DCM_MVP_INVALID_PARAM` |
| Response capacity is too small | No response bytes are written | `DCM_MVP_BUFFER_TOO_SMALL` |

Diagnostic negative responses are protocol results, not API failures. This distinction is reflected in the API status.

## 8. Data and memory handling

- The caller owns all request and response buffers and provides their lengths/capacities.
- The module uses static or constant configuration for DID payloads and does not allocate memory dynamically.
- Check pointers before dereferencing them, and check request length before indexing the request.
- Check capacity for the entire response before writing its first byte.
- Keep fixed response lengths and payload sizes represented by named constants or configuration metadata.
- Use only clearly fictional test data. Treat VINs and other vehicle-linked identifiers as sensitive; do not place production data or secrets in source, tests, logs, or documentation.

## 9. Verification design

Implement a self-contained test harness or use Unity only after the team confirms that dependency. At minimum, test:

- Positive responses for both supported DIDs, including all response bytes and exact lengths.
- Unsupported DID response (`7F 22 31`).
- Request lengths shorter and longer than three bytes (`7F 22 13`).
- Incorrect SID (`7F 22 13`).
- Null request, response, and response-length pointers.
- Insufficient capacity for both positive and negative responses, asserting no response-buffer write occurs.
- Exact minimum capacity for each response type.

Build with C99 mode and the compiler warning settings selected by the team. Compiler, build system, test framework/version, and CI tool versions remain undecided; document the selected versions in build instructions/configuration rather than assuming them here.

## 10. Requirement traceability

| Requirement | Design element | Verification |
|---|---|---|
| One DID, exactly two bytes after SID | Request format and length validation | Short/long request tests; supported DID tests |
| Supported DIDs `0xF190` and `0xF187` | Fixed DID configuration and positive response builder | Positive response byte/length tests |
| Unsupported DID returns NRC `0x31` | DID lookup miss path | Unsupported DID test |
| Invalid length or SID returns NRC `0x13` | Request validation path | Malformed length and incorrect SID tests |
| Null pointers and insufficient capacity reported | API parameter and capacity checks | Null-pointer and insufficient-capacity tests |
| No out-of-bounds writes; deterministic; no dynamic allocation | Capacity-before-write rule and static configuration | Boundary tests and code review |

## 11. Decisions still required

- Select a compiler, build system, warning level, and supported host platforms.
- Select Unity or a self-contained test harness; record exact dependency versions if applicable.
- Agree on the fictional test payloads for both DIDs and how they are represented in C.
- Confirm the test runner and CI workflow/tool versions.
- Agree any broader naming, formatting, or static-analysis standard.

These decisions do not change the requirement-defined response behavior. Keep the implementation within the standalone training scope until requirements explicitly expand it.