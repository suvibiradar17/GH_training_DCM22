# Software Requirements: Diagnostic DID Read MVP

**Project:** AUTOSAR DCM / Diagnostic Communication Software Training MVP  
**Status:** Training baseline  
**Primary requirement:** DCM-MVP-001  
**Sources:** [Training brief](AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md), [project instructions](../.github/copilot-instructions.md), and [software design description](design.md)

## 1. Purpose

This document defines the requirements and acceptance criteria for a standalone training simulator of a simplified UDS ReadDataByIdentifier (`0x22`) request. Requirements describe the training MVP only. They do not define a complete ISO 14229 implementation or a production AUTOSAR DCM component.

## 2. Scope

### 2.1 In scope

- A deterministic C99 module that processes one request containing one DID.
- Fixed configuration for two DID payloads and construction of positive or negative responses.
- Caller-provided input/output buffers, explicit validation, and unit tests for normal, negative, and boundary cases.
- Local-PC build and test workflow, peer review, CI on pull requests, and versioned release packaging as a training deliverable.

### 2.2 Out of scope

- AUTOSAR configuration (ARXML), generated DCM integration, or a production AUTOSAR architecture.
- ECU operation, CAN/CANoe communication, ISO-TP, PduR/CanTp, or diagnostic tester connectivity.
- Session control, security access, authentication, access-control logic, or reads from real ECU signals/NvM.
- Hardware-in-the-loop testing or deployment/flashing to a vehicle or production ECU.

## 3. Assumptions and constraints

- The implementation language is portable C99.
- The target is a developer PC; an ECU and full AUTOSAR stack are not required.
- The public API follows the interface proposal in the training brief and uses fixed-width integer types.
- The caller owns and provides request and response buffers.
- No dynamic memory allocation is used.
- Unity is a suggested test framework, not a selected dependency; a self-contained harness is also acceptable.
- Compiler, build system, compiler version, CI tool versions, and broader coding-style rules have not yet been selected. Do not assume specific tools or versions.
- DID payload contents are not defined here. Test/configuration values must be synthetic and clearly fictional.

## 4. Functional requirements

### FR-01: Accept one DID request

The module shall accept a request containing exactly three bytes: service ID `0x22`, DID high byte, and DID low byte. The DID shall be decoded in network byte order, with the high byte first.

### FR-02: Return data for supported DID `0xF190`

For a valid request for DID `0xF190`, the module shall return a positive response containing SID `0x62`, the requested DID bytes `0xF1 0x90`, and the configured 17-byte ASCII test payload, in that order.

### FR-03: Return data for supported DID `0xF187`

For a valid request for DID `0xF187`, the module shall return a positive response containing SID `0x62`, the requested DID bytes `0xF1 0x87`, and the configured 10-byte ASCII test payload, in that order.

### FR-04: Reject unsupported DIDs

For a three-byte request with SID `0x22` and a DID not configured by the module, it shall return the negative response bytes `0x7F 0x22 0x31` (Request Out Of Range).

### FR-05: Reject malformed request length

For a request whose length is not exactly three bytes, the module shall return the negative response bytes `0x7F 0x22 0x13` (Incorrect Message Length Or Invalid Format). The module shall not access request bytes beyond the supplied request length.

### FR-06: Reject an incorrect service ID

For a three-byte request whose service ID is not `0x22`, the module shall return the negative response bytes `0x7F 0x22 0x13` (Incorrect Message Length Or Invalid Format), as specified by this training requirement.

### FR-07: Report API parameter errors

The request-processing API shall return `DCM_MVP_INVALID_PARAM` if any required pointer (`request`, `response`, or `responseLength`) is null. If `responseLength` is non-null when an API error is reported, the module shall set it to zero.

### FR-08: Report insufficient output capacity

The request-processing API shall return `DCM_MVP_BUFFER_TOO_SMALL` when the provided response capacity cannot contain the complete response. It shall set `responseLength` to zero when that pointer is valid, and shall not write any response bytes.

### FR-09: Report successful response length

When a positive or negative diagnostic response is produced, the API shall return `DCM_MVP_OK` and set `responseLength` to the exact number of response bytes written. The response lengths shall be 20 bytes for DID `0xF190`, 13 bytes for DID `0xF187`, and 3 bytes for a negative response.

## 5. Quality and implementation requirements

### QR-01: Prevent out-of-bounds access and writes

The module shall validate pointers before dereferencing them, validate request length before indexing request data, and check capacity for the complete response before writing its first byte. No response write may exceed `responseCapacity`.

### QR-02: Deterministic behavior

For the same request, configuration, and buffer capacity, the module shall produce the same API status, response bytes, and response length.

### QR-03: No dynamic allocation

The module shall not dynamically allocate memory. All caller data shall use caller-provided buffers; DID configuration shall use fixed/static data.

### QR-04: Portable C99 and explicit protocol constants

Production code shall be written in portable C99. Protocol identifiers, negative response codes, and fixed lengths shall be represented with named constants rather than unexplained numeric literals. Protocol data and public interface values shall use fixed-width integer types.

### QR-05: Synthetic and sensitive-data-safe fixtures

Tests and sample configuration shall use clearly fictional synthetic values only. Real VINs, customer/vehicle data, credentials, tokens, private keys, security-access secrets, and production diagnostic data shall not be committed or placed in logs/documentation. Treat VINs and other vehicle-linked identifiers as sensitive unless explicitly classified otherwise.

## 6. Interface requirement

The proposed public interface is:

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

The project shall preserve the distinction between API errors (`DCM_MVP_INVALID_PARAM`, `DCM_MVP_BUFFER_TOO_SMALL`) and valid diagnostic negative responses, which return `DCM_MVP_OK` when successfully written.

## 7. Verification requirements

Use Unity only if the team selects it; otherwise use a self-contained test harness. Tests shall verify response bytes, API status, exact response length, and buffer boundaries. Capacity-error tests shall verify that the response buffer is not modified (for example, by initializing it with sentinel bytes and checking those bytes afterward).

| Test ID | Requirement(s) | Scenario | Expected result |
|---|---|---|---|
| UT-01 | FR-02, FR-09 | Request DID `0xF190` | `DCM_MVP_OK`; `0x62`, DID, and 17 configured bytes; length 20 |
| UT-02 | FR-03, FR-09 | Request DID `0xF187` | `DCM_MVP_OK`; `0x62`, DID, and 10 configured bytes; length 13 |
| UT-03 | FR-04, FR-09 | Request unsupported DID | `DCM_MVP_OK`; response `7F 22 31`; length 3 |
| UT-04 | FR-05 | Request shorter than 3 bytes | `DCM_MVP_OK`; response `7F 22 13`; length 3 |
| UT-05 | FR-05 | Request longer than 3 bytes | `DCM_MVP_OK`; response `7F 22 13`; length 3 |
| UT-06 | FR-06 | Request with incorrect SID | `DCM_MVP_OK`; response `7F 22 13`; length 3 |
| UT-07 | FR-07 | Null request pointer | `DCM_MVP_INVALID_PARAM`; response length zero when valid |
| UT-08 | FR-07 | Null response pointer | `DCM_MVP_INVALID_PARAM`; response length zero |
| UT-09 | FR-07 | Null response-length pointer | `DCM_MVP_INVALID_PARAM` |
| UT-10 | FR-08, QR-01 | Capacity too small for a supported-DID response | `DCM_MVP_BUFFER_TOO_SMALL`; length zero; no response bytes changed |
| UT-11 | FR-08, QR-01 | Capacity too small for a negative response | `DCM_MVP_BUFFER_TOO_SMALL`; length zero; no response bytes changed |
| UT-12 | FR-08, FR-09, QR-01 | Capacity exactly matches required response length | Successful complete response; no memory overwrite |

## 8. Project delivery acceptance criteria

These criteria cover the training deliverable as well as the software module:

### DA-01: Documented requirement and design

The project shall document assumptions, the public API, and response formats in the requirements and design documents.

### DA-02: Reproducible build and tests

The module shall build with the compiler and warning settings agreed by the team. README shall explain how to build, test, and run the module locally. Selected compiler, build, test, and CI tool versions shall be documented or pinned in project configuration.

### DA-03: Review

A peer review shall be completed. Review findings shall be addressed or recorded, including review of protocol behavior, buffer safety, pointer handling, dynamic allocation, test quality, and assumptions.

Review evidence for the unit-test-design review is recorded in [review-report.md](review-report.md) and [review-findings.csv](review-findings.csv).

### DA-04: CI

GitHub Actions shall build the module and run unit tests on every pull request. Test output shall be published as a workflow artifact.

### DA-05: Release package

On a version tag, the pipeline shall package source, README, requirement/design documents, test evidence, and a version identifier into a versioned ZIP artifact. This is software package publication, not ECU deployment.

## 9. Requirement traceability

| Source | Derived requirement(s) |
|---|---|
| DCM-MVP-001 request format and supported DIDs | FR-01, FR-02, FR-03 |
| DCM-MVP-001 response behavior | FR-04, FR-05, FR-06, FR-09 |
| DCM-MVP-001 interface notes | FR-07, FR-08, FR-09 |
| Project coding instructions | QR-01 through QR-05 |
| Training brief test matrix and definition of done | UT-01 through UT-12, DA-01 through DA-05 |

## 10. Decisions and clarifications pending

- Select the compiler, build system, warning settings, host platforms, and CI tool versions.
- Select Unity or a self-contained unit-test harness and record exact versions for selected dependencies.
- Choose fictional fixed payload values for both DIDs; do not infer or copy a real VIN or production identifier.
- Agree any broader naming, formatting, and static-analysis standard.
- The brief specifies the same negative response for a wrong SID and an invalid length. Preserve this training behavior; do not replace it with behavior inferred from a broader standard without an approved requirement change.