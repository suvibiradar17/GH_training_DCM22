# GitHub Copilot Training – AUTOSAR DCM/Diagnostic SW Developer MVP

## 1. Training overview

**Audience:** AUTOSAR DCM / diagnostic communication software developers  
**Total duration:** 16 hours (concepts + hands-on)  
**Suggested team size:** 2–4 participants per group  
**Suggested implementation:** C (C99) with a lightweight unit-test framework such as Unity, or a simple self-contained test harness  
**Execution environment:** Local PC; no ECU, CAN interface, or full AUTOSAR stack required

### Training goal

Use GitHub Copilot throughout a realistic but small software-development workflow:

1. Requirement analysis
2. Software design
3. Automated development
4. Unit testing
5. Refactoring
6. Code review
7. CI/CD
8. Deployment / release packaging

The exercise focuses on activities directly relevant to a DCM software developer. Full vehicle integration, diagnostic tester communication, and production ECU deployment are intentionally out of scope.

---

## 2. MVP use case: Diagnostic DID Read Service simulator

### Problem statement

A diagnostic software component needs to support a simplified version of **ReadDataByIdentifier (UDS service 0x22)**. For training, implement a standalone C module that accepts a request containing a DID and returns the configured data or an appropriate negative response.

The module simulates the DCM service-processing logic. It does not implement ISO-TP, CAN communication, session/security management, or integration with an AUTOSAR BSW stack.

### Why this use case?

- Familiar to diagnostic software developers.
- Small enough to complete in a 16-hour workshop.
- Supports clear positive and negative test cases.
- Allows Copilot to assist with requirements, C code, tests, review, and pipeline configuration.
- Runs on a developer PC, making the training independent of ECU availability.

---

## 3. Simple software requirement

### Requirement ID: DCM-MVP-001

**Title:** Process a simplified ReadDataByIdentifier (0x22) request.

**Requirement:**

The diagnostic DID handler shall process a request containing UDS service ID `0x22` and one 2-byte Data Identifier (DID).

The handler shall:

1. Accept exactly one DID in the request payload, represented as two bytes following the SID.
2. Support the configured DIDs:
   - `0xF190` – Vehicle Identification Number (VIN), configured as a fixed 17-byte ASCII test value.
   - `0xF187` – Vehicle spare-part number, configured as a fixed 10-byte ASCII test value.
3. For a supported DID, return a positive response consisting of:
   - Positive response SID `0x62`
   - The requested DID (2 bytes)
   - The configured DID data
4. For an unsupported DID, return a negative response:
   - `0x7F 0x22 0x31` (Request Out Of Range)
5. For an invalid request length or incorrect SID, return:
   - `0x7F 0x22 0x13` (Incorrect Message Length Or Invalid Format)
6. Return a clear error status if input/output pointers are null or the output buffer is too small. The implementation shall not write beyond the provided output buffer.
7. Be deterministic and free of dynamic memory allocation.

### Interface proposal

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

**Interface notes:**

- `request` contains the SID followed by the 2-byte DID, so a valid request length is exactly 3 bytes.
- The DID is encoded in network byte order (high byte first).
- On a valid positive or negative diagnostic response, return `DCM_MVP_OK` and set `responseLength`.
- For invalid pointers or insufficient response capacity, return the corresponding API status. Set `responseLength` to zero when it is valid to do so.
- The caller provides all buffers; the module does not allocate memory.

### Example requests and responses

| Request bytes | Expected response | Meaning |
|---|---|---|
| `22 F1 90` | `62 F1 90` + 17 VIN bytes | Supported VIN DID |
| `22 F1 87` | `62 F1 87` + 10 part-number bytes | Supported part-number DID |
| `22 F1 99` | `7F 22 31` | Unsupported DID |
| `22 F1` | `7F 22 13` | Invalid length |
| `19 F1 90` | `7F 22 13` | Incorrect SID |

**Training clarification:** This is a deliberately simplified training requirement, not a complete ISO 14229 or AUTOSAR DCM specification. The team should identify and document any ambiguity during requirements analysis before coding.

---

## 4. Scope and boundaries

### In scope

- Requirement clarification and acceptance criteria
- C module design and implementation
- DID lookup and response construction
- Unit tests, including boundary and negative tests
- Static analysis or compiler warnings where available
- Peer/code review with Copilot assistance
- GitHub repository and pull-request workflow
- GitHub Actions CI pipeline
- Versioned release artifact (for example, a ZIP containing source, test results, and README)

### Out of scope / not required for this developer MVP

- AUTOSAR configuration (ARXML) and generated DCM integration
- ECU, CAN, CANoe, or diagnostic tester connectivity
- ISO-TP and PduR/CanTp integration
- Diagnostic session control, security access, authentication, or access-control logic
- DID read callbacks to real ECU signals or NvM
- Hardware-in-the-loop testing
- Deployment to a vehicle or production ECU

**Optional extension:** If the team finishes early, add a configurable DID table or a simple DID read callback interface.

---

## 5. Definition of Done

The MVP is complete when:

- The requirement and assumptions are documented.
- The public API and response format are documented.
- The module builds with the agreed compiler and warning settings.
- Positive, negative, malformed-request, null-pointer, and insufficient-buffer tests pass.
- No out-of-bounds write occurs in the tested scenarios.
- A peer review is completed and findings are addressed or recorded.
- GitHub Actions builds and runs the unit tests on every pull request.
- A tagged release or versioned package is produced by the pipeline.
- README explains how to build, test, and run the module locally.

---

## 6. Suggested 16-hour agenda

The agenda totals **16 hours**. It assumes participants have basic C and Git knowledge and access to GitHub Copilot.

| Session | Concept | Hands-on activity | Duration |
|---|---|---|---:|
| 1. Kickoff and Copilot fundamentals | Copilot capabilities, limitations, prompt context, responsible use, verification | Set up repository, review the requirement, ask Copilot to identify ambiguities and acceptance criteria | 1.5 h |
| 2. Requirement analysis | Requirement decomposition, edge cases, traceability, acceptance criteria | Create a requirement checklist and testable acceptance criteria; review Copilot output against the requirement | 2.0 h |
| 3. Software design | API design, data flow, error handling, separation of concerns | Ask Copilot for a small design proposal; agree interface, response format, and DID data representation | 1.5 h |
| 4. Develop with Copilot | Context-rich prompts, incremental implementation, code explanation | Implement the module in small steps; compile frequently and inspect generated code | 3.0 h |
| 5. Unit testing | Test design, positive/negative/boundary tests, test coverage | Generate and refine unit tests; run tests and fix defects | 2.5 h |
| 6. Refactor and code review | Readability, defensive C, maintainability, review checklists | Refactor duplicated logic; conduct a peer review and use Copilot to explain or identify potential issues | 1.5 h |
| 7. CI/CD and release packaging | CI concepts, pull requests, build/test automation, artifacts | Create GitHub Actions workflow to build, test, and package the MVP | 2.0 h |
| 8. Demo and retrospective | Demonstration, lessons learned, safe use of AI-generated code | Demonstrate requirement-to-release traceability and discuss improvements | 2.0 h |
| **Total** |  |  | **16.0 h** |

---

## 7. Hands-on tasks by lifecycle stage

### A. Requirement analysis

Ask Copilot to:
- Identify ambiguous or missing details in DCM-MVP-001.
- Convert the requirement into measurable acceptance criteria.
- Propose positive, negative, and boundary test scenarios.
- Create a requirement-to-test traceability table.

**Expected output:** `requirements.md` with assumptions, acceptance criteria, and test scenarios.

**Trainer checkpoint:** Confirm request length, byte order, NRC values, and output-buffer behavior before implementation.

### B. Design

Ask Copilot to propose:
- A minimal module decomposition.
- The public API and status handling.
- A response construction flow.
- A fixed DID table without dynamic memory allocation.

**Expected output:** `design.md` and a documented header file.

**Trainer checkpoint:** Ensure the design remains standalone and does not imply full AUTOSAR DCM conformance.

### C. Automate / develop

Implement in small, reviewable increments:

1. Define API types and constants.
2. Validate pointers and request length.
3. Validate SID and decode DID.
4. Look up the DID.
5. Construct positive or negative response.
6. Check response capacity before writing.
7. Compile and run tests after each increment.

**Expected output:** `src/dcm_mvp.c` and `include/dcm_mvp.h`.

### D. Test

Minimum test list:

| Test ID | Scenario | Expected result |
|---|---|---|
| UT-01 | Read supported DID `0xF190` | Positive response with SID `0x62`, DID, and 17 data bytes |
| UT-02 | Read supported DID `0xF187` | Positive response with SID `0x62`, DID, and 10 data bytes |
| UT-03 | Read unsupported DID | NRC `0x31` |
| UT-04 | Request shorter than 3 bytes | NRC `0x13` |
| UT-05 | Request longer than 3 bytes | NRC `0x13` |
| UT-06 | Incorrect SID | NRC `0x13` |
| UT-07 | Null request pointer | API returns invalid parameter |
| UT-08 | Null response pointer | API returns invalid parameter |
| UT-09 | Null response-length pointer | API returns invalid parameter |
| UT-10 | Output capacity too small for positive response | API returns buffer-too-small; no out-of-bounds write |
| UT-11 | Output capacity too small for negative response | API returns buffer-too-small; no out-of-bounds write |
| UT-12 | Exact minimum capacity | Successful response without memory overwrite |

**Expected output:** Unit-test source and recorded test results.

### E. Refactor

Use Copilot to identify:
- Repeated response-building logic.
- Magic numbers that should become named constants.
- Unclear names or overly complex functions.
- Missing const correctness or defensive checks.

Refactor without changing externally observable behavior. Re-run all tests after each meaningful change.

**Expected output:** Refactored code and passing regression tests.

### F. Review

Conduct a peer review using this checklist:

- Does the code match every requirement and acceptance criterion?
- Are positive and negative response bytes correct?
- Are all buffer lengths checked before writing?
- Are null pointers handled consistently?
- Is there any dynamic memory allocation?
- Are there hidden assumptions or unreviewed Copilot-generated changes?
- Are tests meaningful and independent of implementation details?

**Expected output:** Pull-request review comments and resolved or documented findings.

### G. CI/CD

Create a GitHub Actions workflow that runs on pull requests and pushes to the main branch.

Pipeline stages:

1. Check out repository.
2. Configure compiler/build environment.
3. Compile with warnings enabled.
4. Run unit tests.
5. Publish test output as a workflow artifact.
6. On a version tag, package source, README, and test evidence into a versioned ZIP artifact.

**Expected output:** A passing GitHub Actions workflow and a downloadable release artifact from the workflow run.

**Note:** Use the compiler and test framework available in the training environment. Pin or document tool versions so results are reproducible.

### H. Deployment / release

For this embedded software developer MVP, “deployment” means **publishing a versioned software package**, not flashing an ECU.

Package contents:
- `src/` and `include/`
- README with build and test instructions
- Requirement and design documents
- Test summary
- Version identifier

**Expected output:** A tagged version such as `v1.0.0` and a CI-generated ZIP release artifact.

---

## 8. Suggested repository structure

```text
dcm-did-mvp/
├── .github/
│   └── workflows/
│       └── ci.yml
├── docs/
│   ├── requirements.md
│   ├── design.md
│   └── test-summary.md
├── include/
│   └── dcm_mvp.h
├── src/
│   └── dcm_mvp.c
├── tests/
│   └── test_dcm_mvp.c
├── README.md
└── .gitignore
```

---

## 9. Suggested Copilot prompts

Use these as starting points; participants should provide the requirement and relevant source files as context.

**Requirement analysis**

> Review DCM-MVP-001 as an embedded C diagnostic software developer. List ambiguities, assumptions, and missing acceptance criteria. Do not invent AUTOSAR or ISO requirements; clearly label suggestions.

**Design**

> Propose a minimal, deterministic C99 design for the attached requirement. Use caller-provided buffers, no dynamic memory allocation, and explicit output-capacity checks. Explain trade-offs before generating code.

**Implementation**

> Implement only the request-validation portion of DcmMvp_ProcessRequest based on the agreed interface. Keep the change small, explain edge cases, and do not modify unrelated files.

**Unit testing**

> Create table-driven unit tests for the documented acceptance criteria. Include positive, negative, malformed-input, null-pointer, and buffer-boundary cases. Do not change production code to make a failing test pass without explaining why.

**Refactoring**

> Review this C module for maintainability and defensive-programming improvements. First list proposed changes and risks. Preserve the public API and externally observable behavior.

**Code review**

> Review the diff against DCM-MVP-001 and the acceptance criteria. Report findings by severity with file/line references where possible. Focus on correctness, memory safety, and missing tests. Do not make changes automatically.

**CI workflow**

> Draft a GitHub Actions workflow for this repository that builds the C module, runs unit tests on pull requests, and packages a versioned ZIP on version tags. State any assumptions about the compiler and test framework.

---

## 10. Training evaluation

Assess participants on the workflow and engineering judgment, not on how much code Copilot generates.

| Area | Evidence |
|---|---|
| Requirement analysis | Clear acceptance criteria and documented assumptions |
| Design | Simple API, data flow, and error handling |
| Development | Incremental implementation with human verification |
| Testing | Coverage of required positive, negative, and boundary cases |
| Refactoring | Improved clarity without regression |
| Review | Useful findings and evidence that findings were addressed |
| CI/CD | Automated build and test pipeline |
| Release | Reproducible, versioned package and usage instructions |

### Key takeaway

GitHub Copilot is an assistant, not the design authority or safety approver. Developers remain responsible for requirement interpretation, AUTOSAR/UDS correctness, memory safety, code review, and validation of generated code.
