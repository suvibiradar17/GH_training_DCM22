---
description: "Design comprehensive unit test cases for the Diagnostic DID Read MVP and save them as CSV."
---

# Unit Test Case Design to CSV

## Role

Act as a senior embedded C test engineer experienced in designing deterministic unit tests for small C99 modules.

## Objective

Create a comprehensive unit-test case design for the Diagnostic DID Read MVP. Use `Docs/requirements.md` as the primary source of truth and `Docs/design.md` as supporting detail for expected behavior, API semantics, response bytes, limits, and scope.

Strict TDD constraint: do not implement or generate production code, unit-test source code, stubs, mocks, fixtures, harness code, or test automation. At this workflow stage, produce design and review artifacts only.

## Coverage requirements

Design cases for all behavior specified in the source, including:

- Happy-path positive responses for every supported DID, validating every response byte and exact response length.
- Negative protocol responses for unsupported DIDs, malformed request lengths, and incorrect service IDs.
- Pointer validation for each required pointer independently.
- Response capacity boundaries: exact required capacity, one byte below required capacity, and representative larger capacity for each distinct response size/class.
- No-write behavior on API errors and insufficient capacity, including response-length reset behavior wherever the design specifies it.
- Request-length boundaries (zero, shorter than three bytes, exactly three bytes, and longer than three bytes) without assuming access to bytes outside the provided length.
- DID decoding in high-byte-first order, including supported identifiers and an unsupported identifier.
- Deterministic behavior for identical calls.

Be exhaustive across the documented behaviors, input classes, and meaningful boundaries; do not claim mathematical exhaustiveness beyond the defined scope. Add a case only when its expected result follows from `Docs/requirements.md` and is supported by `Docs/design.md`. Do not invent protocol rules, statuses, supported DIDs, payload values, or requirements from AUTOSAR/ISO documents or other files.

If a test needs configured DID data whose exact bytes are not specified, identify the fixture as a clearly fictional implementation-configured payload and express expected bytes as the configured payload. Do not invent or include a real VIN or vehicle-linked data.

## CSV output

Create or overwrite this output file:

`Docs/ut-design-testcase.csv`

The CSV must have exactly these columns, in this order, using these header names:

1. `testcase id`
2. `testcase name`
3. `test description`
4. `clear input`
5. `testcase type`
6. `expected output`
7. `actual output`
8. `automated (yes or no)`
9. `comments`

## CSV content rules

- Produce valid UTF-8 CSV with one header row and one test case per row.
- Quote fields when they contain commas, quotation marks, or line breaks; escape embedded quotation marks according to CSV rules.
- Give every test case a unique, stable ID (for example, `UT-001`, `UT-002`).
- Make `clear input` precise enough to reproduce the case. Include API arguments, request bytes and length, response capacity, pointer-null condition if applicable, and relevant initial buffer/length sentinel values. Do not put multiple distinct scenarios in one row.
- Use meaningful `testcase type` values such as `positive`, `negative`, `edge`, `boundary`, `error handling`, and `determinism`.
- In `expected output`, state the API status, exact response bytes or byte pattern, exact response length, and whether output buffers must remain unchanged, as applicable.
- Leave `actual output` empty because no test execution is being performed. Do not predict or fabricate observed results.
- Set `automated (yes or no)` to `Yes` when the case is suitable for deterministic unit-test automation, and `No` only when it inherently requires manual evaluation. This indicates automation suitability, not that automation already exists.
- Use `comments` for assumptions or notes, including payload fixtures that must be selected by the implementation team.
- Do not include Markdown fences, explanatory prose, or a summary in the CSV file. The CSV itself must be the deliverable.

## Final checks

Before saving, verify that:

- Every response path and API error path in `Docs/requirements.md` has one or more test cases.
- Both supported DID response sizes and all negative response lengths are covered at exact and insufficient capacity boundaries.
- Every requested CSV column appears exactly once and in the required order.
- All rows have the same number of columns and valid CSV escaping.
- The file contains no production implementation, no test implementation, and no fabricated actual results.