# Project Instructions: AUTOSAR DCM Training MVP

## 1. Project name

**AUTOSAR DCM / Diagnostic Communication Software Training MVP**  
The hands-on use case is a standalone **Diagnostic DID Read Service simulator**.

## 2. Description

This project is a training implementation of a simplified UDS ReadDataByIdentifier service (`0x22`). It accepts a request for one configured DID and returns either its configured data or a negative response.

This is a standalone developer-PC simulator. It is not a complete ISO 14229 implementation or a production AUTOSAR DCM component. Do not assume ECU integration, CAN/ISO-TP, AUTOSAR configuration, sessions, or security unless a requirement explicitly asks for them.

The project requirements and training scope are documented in [AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md](../Docs/AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md).

## 3. Technology stack

- Language: C99.
- Unit testing: Unity is suggested, but the project has not selected a test framework. A self-contained test harness is also acceptable.
- Target environment: Local PC; no ECU or full AUTOSAR stack is required.
- Build system, compiler, compiler version, and CI tool versions: **Not yet selected. Do not assume these.**

## 4. Design pattern

No formal design pattern has been selected.

Follow the simple design described by the requirements:
- Keep request processing deterministic and use caller-provided buffers.
- Use a fixed DID lookup table or equivalent simple lookup.
- Keep the public interface small and separate request validation, DID lookup, and response construction where that improves clarity.
- Do not introduce a larger framework or claim AUTOSAR architectural conformance.

## 5. Language versions, libraries, and sources

- C language version: **C99**, as specified in the training brief.
- Test library: **Unity is a suggestion, not a confirmed dependency**. Confirm the team's choice before relying on it.
- No other external libraries or versions are currently specified.
- Source of project requirements and proposed interface: [AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md](../Docs/AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md).
- When adding a dependency or tool, document its exact version and source in build documentation or project configuration. Do not invent version numbers.

## 6. Coding standards

- Write portable C99.
- Avoid dynamic memory allocation; the module must use caller-provided buffers.
- Validate pointers, request lengths, and response capacity before accessing or writing data.
- Never write beyond the provided response capacity.
- Set response length to zero when appropriate on API errors, as specified by the API contract.
- Use fixed-width integer types for protocol data and public interfaces.
- Use named constants for service IDs, response IDs, NRCs, and fixed lengths instead of unexplained numeric literals.
- Keep behavior deterministic and functions focused.
- Preserve the specified UDS response bytes and distinguish API errors from valid diagnostic negative responses.
- Do not claim compliance with the complete ISO 14229 or AUTOSAR specifications based on this training MVP.
- No broader naming, formatting, or static-analysis standard has been selected. Record an agreed standard here when the team chooses one.

## 7. Sample test code

The following is an example Unity test for the proposed API. It assumes the project has chosen Unity and implemented the interface from the training brief. The expected VIN payload is intentionally not specified here; use the configured test value from the implementation or a shared test fixture.

```c
#include <stdint.h>
#include "dcm_mvp.h"
#include "unity.h"

void test_ReadVin_returns_positive_response_header_and_length(void)
{
	const uint8_t request[] = { 0x22u, 0xF1u, 0x90u };
	uint8_t response[20];
	uint16_t responseLength = 0u;

	DcmMvp_StatusType status = DcmMvp_ProcessRequest(
		request,
		(uint16_t)sizeof(request),
		response,
		(uint16_t)sizeof(response),
		&responseLength);

	TEST_ASSERT_EQUAL(DCM_MVP_OK, status);
	TEST_ASSERT_EQUAL_UINT16(20u, responseLength);
	TEST_ASSERT_EQUAL_HEX8(0x62u, response[0]);
	TEST_ASSERT_EQUAL_HEX8(0xF1u, response[1]);
	TEST_ASSERT_EQUAL_HEX8(0x90u, response[2]);
}
```

Add tests for supported and unsupported DIDs, malformed request lengths, incorrect SID, null pointers, and insufficient output capacity. Verify that insufficient capacity does not cause an out-of-bounds write.

## 8. Folder structure

The training brief suggests the following structure. Treat it as a proposed layout, not a statement that every directory or file already exists:

```text
dcm-did-mvp/
├── .github/
│   └── workflows/
│       └── ci.yml
├── Docs/
│   ├── requirements.md
│   └── design.md
├── include/
│   └── dcm_mvp.h
├── src/
│   └── dcm_mvp.c
├── tests/
│   └── test_dcm_mvp.c
├── README.md
└── [build configuration to be selected]
```

Keep production code, public headers, tests, and documentation in their respective areas. Update this section if the repository structure changes.

## 9. Sensitive data handling

- Use synthetic test values only. Do not commit real vehicle identification numbers (VINs), customer or vehicle data, credentials, tokens, private keys, security-access secrets, or production diagnostic data.
- Treat VINs and other vehicle-linked identifiers as sensitive unless the project owner explicitly classifies the data otherwise.
- Do not put secrets in source code, tests, logs, documentation, or CI configuration. Use an approved secret store for any future CI secrets.
- Keep test fixtures clearly marked as fictional and ensure they cannot be mistaken for production data.
- Review generated code and test data for accidental sensitive information before committing.
