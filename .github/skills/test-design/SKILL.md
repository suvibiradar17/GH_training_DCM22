---
name: test-design
description: Design requirement-based unit test artifacts for the Diagnostic DID Read MVP without generating implementation code.
---

# Test Design Skill

## Purpose

Use this skill to automate **test design artifacts only** for the AUTOSAR DCM Diagnostic DID Read MVP.

This skill is intended for TDD-oriented work where the next step is to design tests and review artifacts before any production implementation is created.

## When to use

Use this skill when you need to:

- derive unit-test scenarios from [Docs/requirements.md](../../../Docs/requirements.md)
- refine or regenerate [Docs/unit-test-design.md](../../../Docs/unit-test-design.md)
- create or update [Docs/ut-design-testcase.csv](../../../Docs/ut-design-testcase.csv)
- refresh review artifacts such as [Docs/review-report.md](../../../Docs/review-report.md), [Docs/review-findings.csv](../../../Docs/review-findings.csv), and [Docs/review-decisions.json](../../../Docs/review-decisions.json)
- keep requirement traceability and boundary coverage consistent after requirement or design changes

## Required inputs

Review these sources before making changes:

1. [Docs/requirements.md](../../../Docs/requirements.md)
2. [Docs/design.md](../../../Docs/design.md)
3. [Docs/AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md](../../../Docs/AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md)
4. Existing review and test-design artifacts, when present

## Strict TDD constraint

**DO NOT create, modify, or suggest actual production code implementation, unit-test source code, stubs, mocks, fixtures, harness code, headers, C source files, or executable test logic while using this skill.**

**This repository is following TDD for this workflow stage. The deliverables at this stage are design and review artifacts only.**

Allowed outputs for this skill are limited to documents and structured review artifacts such as:

- `Docs/unit-test-design.md`
- `Docs/ut-design-testcase.csv`
- `Docs/review-report.md`
- `Docs/review-findings.csv`
- `Docs/review-decisions.json`

## Expected workflow

1. Read the requirements first and treat them as the primary source of truth.
2. Use the design document only as supporting detail.
3. Identify all required functional, negative, boundary, and determinism scenarios.
4. Preserve explicit traceability from test cases to requirements.
5. Record review findings and decisions when gaps are found.
6. If review automation is available, refresh the generated review artifacts after updating the design artifacts.

## Coverage expectations

Ensure the design covers, when required by the inputs:

- supported DID positive responses
- unsupported DID negative responses
- malformed request length handling
- incorrect SID handling
- null-pointer API errors
- insufficient-capacity behavior
- exact-capacity boundaries
- zero-capacity boundary when useful for no-write verification
- DID byte-order verification
- deterministic repeated-call behavior
- non-functional review items that must be verified later by review or static analysis

## Output quality rules

- Do not invent behavior beyond the documented requirements.
- Do not invent real VINs or real vehicle-linked data.
- Use fictional synthetic payload wording where payload bytes are not fixed yet.
- Keep CSV structure stable and machine-readable.
- Leave `actual output` empty in design CSV artifacts.
- Distinguish review findings from review decisions.
- Keep review follow-up items explicit when they remain open.

## Completion criteria

This skill is complete only when:

- the test design is requirement-aligned
- the CSV test matrix is updated consistently
- review artifacts are synchronized with the latest design state
- no implementation code has been created or modified
