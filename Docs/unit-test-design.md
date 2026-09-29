# Unit Test Case Design: Diagnostic DID Read MVP

**Status:** Design only; no test code or automation included  
**System under test:** `DcmMvp_ProcessRequest`  
**Primary source of truth:** [Software Requirements](requirements.md)  
**Supporting design source:** [Software Design Description](design.md)

## 1. Purpose

Define the unit-test cases for the standalone DID Read MVP. The cases verify the API status, response bytes, response length, and buffer behavior specified by the requirements and refined by the design. They do not define behavior beyond those sources.

## 2. Test scope

Test one request at a time through the public `DcmMvp_ProcessRequest` interface. Cover:

- Positive responses for configured DIDs `0xF190` and `0xF187`.
- Negative responses for unsupported DIDs, malformed request lengths, and incorrect service IDs.
- DID decoding in network byte order, with the high byte first.
- Null-pointer API errors.
- Insufficient response capacity and exact-capacity boundaries.
- Response-length reporting, response-buffer non-modification on API/capacity errors, and deterministic repeated calls.

No CAN, ISO-TP, ECU, AUTOSAR BSW, session, or security behavior is tested.

## 3. Test setup and data

- Call the public API with caller-owned request and response buffers.
- Initialize `responseLength` to a non-zero sentinel before each applicable call, so tests can verify when the API resets it to zero or sets the final length.
- Fill the response buffer and guard bytes with a recognizable sentinel before calls that must not write, especially insufficient-capacity and invalid-pointer cases.
- Configure the implementation and expected results with the same clearly fictional test payload for each supported DID. Payloads are not specified by the requirements or design; select and record them when implementation begins. Do not use real VINs or vehicle-linked data.
- For positive responses, expected bytes are the positive SID (`0x62`), the DID high byte, the DID low byte, and the configured payload bytes in that order.
- For negative responses, expected bytes are `0x7F 0x22` followed by the applicable NRC.
- Include representative larger-capacity cases and at least one zero-capacity case to confirm complete-response sizing logic and no-write behavior.

## 4. Test cases

| ID | Objective and input | Capacity / setup | Expected result |
|---|---|---|---|
| UT-01 | Read supported DID `0xF190` using request `22 F1 90`. | Capacity exactly 20 bytes; configured fictional 17-byte payload. | `DCM_MVP_OK`; response `62 F1 90` followed by all 17 configured bytes; response length 20. |
| UT-02 | Read supported DID `0xF187` using request `22 F1 87`. | Capacity exactly 13 bytes; configured fictional 10-byte payload. | `DCM_MVP_OK`; response `62 F1 87` followed by all 10 configured bytes; response length 13. |
| UT-03 | Read an unconfigured DID, for example `0xF199`, using SID `0x22`. | Capacity exactly 3 bytes. | `DCM_MVP_OK`; response `7F 22 31`; response length 3. |
| UT-04 | Submit a request shorter than 3 bytes. Exercise lengths 0, 1, and 2 with a non-null request pointer. | Capacity exactly 3 bytes for each subcase. | Each call returns `DCM_MVP_OK`; response `7F 22 13`; response length 3. No request byte beyond the supplied length is accessed. |
| UT-05 | Submit a request longer than 3 bytes, including a trailing byte after an otherwise valid request. | Capacity exactly 3 bytes. | `DCM_MVP_OK`; response `7F 22 13`; response length 3. The extra request byte does not cause the request to be accepted. |
| UT-06 | Submit a three-byte request with a SID other than `0x22`, for example `19 F1 90`. | Capacity exactly 3 bytes. | `DCM_MVP_OK`; response `7F 22 13`; response length 3. |
| UT-07 | Pass a null request pointer and otherwise valid output arguments. | Set `responseLength` to a non-zero sentinel and prefill response with sentinels. | `DCM_MVP_INVALID_PARAM`; response length becomes zero; response buffer remains unchanged. |
| UT-08 | Pass a null response pointer and otherwise valid request and response-length arguments. | Set `responseLength` to a non-zero sentinel. | `DCM_MVP_INVALID_PARAM`; response length becomes zero. No response is produced. |
| UT-09 | Pass a null `responseLength` pointer with otherwise valid request and response arguments. | Prefill response with sentinels. | `DCM_MVP_INVALID_PARAM`; response buffer remains unchanged. No response length can be asserted because its pointer is null. |
| UT-10 | Request each supported DID with insufficient positive-response capacity. Exercise capacity 19 for `0xF190` and 12 for `0xF187`. | Prefill the entire response storage, including guard bytes, with sentinels; set response length to a non-zero sentinel. | Each call returns `DCM_MVP_BUFFER_TOO_SMALL`; response length becomes zero; no response byte or guard byte changes. |
| UT-11 | Request a negative response with capacity below 3 bytes. Exercise an unsupported DID and at least one malformed-request or incorrect-SID path. | Use capacity 2; prefill response and guard bytes with sentinels; set response length to a non-zero sentinel. | Each call returns `DCM_MVP_BUFFER_TOO_SMALL`; response length becomes zero; no response byte or guard byte changes. |
| UT-12 | Verify exact minimum response capacity for each response class: DID `0xF190`, DID `0xF187`, and one negative-response case. | Use capacities 20, 13, and 3 respectively. | Each call succeeds with `DCM_MVP_OK`, writes the complete expected response only within capacity, and reports the exact length. |
| UT-13 | Repeat identical calls using the same request, configuration, and capacity. Include one supported DID and one negative-response case. | Reset output storage between calls. | Each pair of calls returns identical statuses, response bytes, and response lengths. |
| UT-14 | Submit a three-byte request with the supported DID bytes reversed, for example `22 90 F1`, to verify high-byte-first DID decoding. | Capacity exactly 3 bytes. | `DCM_MVP_OK`; response `7F 22 31`; response length 3, because `0x90F1` is not a configured DID. |

## 5. Buffer-boundary observation

For UT-10 and UT-11, the response storage should include a test-only guard region after the capacity passed to the API. Initialize both the supplied region and guard region to sentinels. On `DCM_MVP_BUFFER_TOO_SMALL`, assert that all sentinel values remain unchanged. For UT-12, confirm the exact-capacity response is complete and guard bytes beyond the supplied capacity remain unchanged.

The design requires the module to check full response capacity before writing any response byte. Therefore, a short-capacity call must not leave a partial response in the supplied buffer.

## 6. Execution and reporting

This document defines test intent only. The team has not selected Unity or a self-contained harness, compiler, build system, or test runner. Once selected, implement these cases in that framework and record the chosen tool/version and test results separately. No CI configuration or test automation is specified here.

Report each case as pass, fail, or blocked, with the observed API status, response length, and relevant response-buffer contents. For payload-dependent cases, record the fictional fixture value used by the implementation and compare every payload byte.

## 7. Requirement and design traceability

| Requirement / behavior | Test case(s) |
|---|---|
| FR-01: Accept one three-byte DID request and decode DID high byte first | UT-01, UT-02, UT-04, UT-05, UT-14 |
| FR-02: Supported DID `0xF190` positive response, 17 data bytes, length 20 | UT-01, UT-10, UT-12, UT-13 |
| FR-03: Supported DID `0xF187` positive response, 10 data bytes, length 13 | UT-02, UT-10, UT-12 |
| FR-04: Unsupported DID response `7F 22 31` | UT-03, UT-11, UT-12, UT-13, UT-14 |
| FR-05: Invalid request length response `7F 22 13` | UT-04, UT-05, UT-11 |
| FR-06: Incorrect SID response `7F 22 13` | UT-06, UT-11 |
| FR-07: Null pointer API error and output-length handling | UT-07, UT-08, UT-09 |
| FR-08 / QR-01: Capacity check before writing; no partial or out-of-bounds response | UT-10, UT-11, UT-12 |
| FR-09: Exact successful response-length reporting | UT-01, UT-02, UT-03, UT-04, UT-05, UT-06, UT-12 |
| QR-02: Deterministic processing | UT-13 |
| QR-05: Fictional synthetic fixtures only | UT-01, UT-02 and all payload-dependent implementation tests |

Detailed CSV rows in [ut-design-testcase.csv](ut-design-testcase.csv) split several aggregate cases above into single-scenario entries to improve reproducibility and one-to-one traceability.

## 8. Verification by review or static analysis

The following requirements are not fully established by functional unit tests alone and shall be verified during implementation review and, where available, static analysis:

| Requirement | Verification approach |
|---|---|
| QR-03: No dynamic allocation | Review production code for absence of heap allocation APIs and confirm DID configuration uses fixed/static storage only. |
| QR-04: Portable C99, fixed-width types, and named protocol constants | Review headers and source for C99-compatible constructs, use of `stdint.h` fixed-width types, and named constants instead of unexplained numeric literals. |