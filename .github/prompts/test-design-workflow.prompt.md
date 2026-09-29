---
description: "Use the test-design skill to update requirement-based test-design and review artifacts for the Diagnostic DID Read MVP without generating implementation code."
---

# Test Design Workflow Prompt

Use the `test-design` skill for this task.

## Objective

Update or regenerate the requirement-based test-design artifacts for the Diagnostic DID Read MVP.

## Mandatory constraints

- Follow TDD for this workflow stage.
- Do **not** create, modify, or suggest production code.
- Do **not** create, modify, or suggest unit-test source code, mocks, stubs, fixtures, harness code, or executable test logic.
- Only update design and review artifacts.

## Required inputs

Read these sources first:

- `Docs/requirements.md`
- `Docs/design.md`
- `Docs/AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md`
- Existing review artifacts, if present

## Expected outputs

Depending on what is out of date, create or update only these artifacts:

- `Docs/unit-test-design.md`
- `Docs/ut-design-testcase.csv`
- `Docs/review-report.md`
- `Docs/review-findings.csv`
- `Docs/review-decisions.json`

## Execution rules

1. Treat `Docs/requirements.md` as the primary source of truth.
2. Use `Docs/design.md` only as supporting detail.
3. Keep one-to-one requirement traceability where practical.
4. Preserve explicit boundary, negative, and determinism coverage.
5. Use fictional synthetic payload wording where payload bytes are not finalized.
6. If available, run repository review automation after updating the artifacts so all review outputs stay synchronized.

## Completion condition

Finish only when all updated design and review artifacts are internally consistent and no implementation code has been produced.
