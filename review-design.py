from __future__ import annotations

import argparse
import csv
import json
import re
from datetime import date
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parent
DEFAULT_DOCS_DIR = REPO_ROOT / "Docs"

REQUIREMENT_IDS = [
    "FR-01",
    "FR-02",
    "FR-03",
    "FR-04",
    "FR-05",
    "FR-06",
    "FR-07",
    "FR-08",
    "FR-09",
    "QR-01",
    "QR-02",
    "QR-03",
    "QR-04",
    "QR-05",
]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def read_csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def extract_requirement_refs(text: str) -> set[str]:
    return set(re.findall(r"\b(?:FR|QR|DA)-\d{2}\b", text))


def csv_rows_with_requirement(rows: Iterable[dict[str, str]], requirement_id: str) -> list[str]:
    testcase_ids: list[str] = []
    for row in rows:
        comments = row.get("comments", "")
        if requirement_id in comments:
            testcase_ids.append(row.get("testcase id", ""))
    return testcase_ids


def find_testcase(rows: Iterable[dict[str, str]], pattern: str) -> dict[str, str] | None:
    regex = re.compile(pattern, re.IGNORECASE)
    for row in rows:
        haystack = " ".join(
            [
                row.get("testcase id", ""),
                row.get("testcase name", ""),
                row.get("test description", ""),
                row.get("clear input", ""),
                row.get("expected output", ""),
                row.get("comments", ""),
            ]
        )
        if regex.search(haystack):
            return row
    return None


def format_testcase_links(testcase_ids: Iterable[str]) -> str:
    ids = [value for value in testcase_ids if value]
    if not ids:
        return "Not covered"
    return ", ".join(ids)


def finding_status(condition: bool) -> str:
    return "Closed" if condition else "Open"


def build_findings(unit_test_design: str, testcase_rows: list[dict[str, str]]) -> list[dict[str, str]]:
    findings: list[dict[str, str]] = []

    requirements_primary = (
        "Primary source of truth:" in unit_test_design
        and "requirements.md" in unit_test_design
        and "Supporting design source:" in unit_test_design
    )
    findings.append(
        {
            "finding id": "RVW-001",
            "title": "Requirements-first traceability",
            "finding description": (
                "The test design should treat the requirements as the primary source of truth "
                "and the design as supporting detail."
            ),
            "severity": "Medium",
            "status": finding_status(requirements_primary),
            "reviewed artifact": "unit-test-design.md",
            "requirement impact": "Affects FR-01 through FR-09 and DA-03",
            "disposition / action taken": (
                "Requirements-first wording is present in the unit-test design."
                if requirements_primary
                else "Update the unit-test design so requirements.md is the primary source of truth."
            ),
            "evidence": "unit-test-design.md",
            "comments": "No further action open." if requirements_primary else "Action required.",
        }
    )

    reversed_did_case = find_testcase(testcase_rows, r"22\s+90\s+F1|Reversed DID byte order") is not None
    high_byte_note = "high byte first" in unit_test_design or "high-byte-first" in unit_test_design
    byte_order_proven = reversed_did_case and high_byte_note
    findings.append(
        {
            "finding id": "RVW-002",
            "title": "Missing DID byte-order proof",
            "finding description": (
                "The review expects an explicit test that proves DID decoding is performed in network byte order."
            ),
            "severity": "Medium",
            "status": finding_status(byte_order_proven),
            "reviewed artifact": "unit-test-design.md; ut-design-testcase.csv",
            "requirement impact": "Affects FR-01 and FR-04",
            "disposition / action taken": (
                "A reversed-byte DID scenario is included and documented."
                if byte_order_proven
                else "Add a reversed-byte DID test such as request 22 90 F1 with unsupported-DID response."
            ),
            "evidence": "unit-test-design.md; ut-design-testcase.csv",
            "comments": "No further action open." if byte_order_proven else "Action required.",
        }
    )

    all_rows_have_req_refs = all(
        "Req:" in row.get("comments", "") for row in testcase_rows if row.get("testcase id", "")
    )
    findings.append(
        {
            "finding id": "RVW-003",
            "title": "Weak requirement traceability in CSV",
            "finding description": (
                "Each CSV test row should carry requirement references to support review and audit traceability."
            ),
            "severity": "Medium",
            "status": finding_status(all_rows_have_req_refs),
            "reviewed artifact": "ut-design-testcase.csv",
            "requirement impact": "Affects DA-03 and requirement traceability",
            "disposition / action taken": (
                "Requirement references are present in the CSV comments field for each test case."
                if all_rows_have_req_refs
                else "Add requirement references to each CSV test row."
            ),
            "evidence": "ut-design-testcase.csv",
            "comments": "No further action open." if all_rows_have_req_refs else "Action required.",
        }
    )

    zero_capacity_case = find_testcase(testcase_rows, r"response capacity is zero|responseCapacity=0") is not None
    wrong_sid_short_capacity = find_testcase(testcase_rows, r"Incorrect-SID NRC capacity one byte too small|request=19 F1 90; requestLength=3; responseCapacity=2") is not None
    stronger_boundaries = zero_capacity_case and wrong_sid_short_capacity
    findings.append(
        {
            "finding id": "RVW-004",
            "title": "Boundary coverage could be stronger",
            "finding description": (
                "Boundary coverage should explicitly include zero-capacity output and wrong-SID insufficient-capacity handling."
            ),
            "severity": "Low",
            "status": finding_status(stronger_boundaries),
            "reviewed artifact": "ut-design-testcase.csv",
            "requirement impact": "Affects FR-08 and QR-01",
            "disposition / action taken": (
                "Dedicated zero-capacity and wrong-SID short-capacity rows are present."
                if stronger_boundaries
                else "Add explicit zero-capacity and wrong-SID insufficient-capacity test rows."
            ),
            "evidence": "ut-design-testcase.csv",
            "comments": "No further action open." if stronger_boundaries else "Action required.",
        }
    )

    review_section_present = "Verification by review or static analysis" in unit_test_design
    qr03_present = "QR-03" in unit_test_design
    qr04_present = "QR-04" in unit_test_design
    non_functional_review_items = review_section_present and qr03_present and qr04_present
    findings.append(
        {
            "finding id": "RVW-005",
            "title": "Non-functional review items not explicit",
            "finding description": (
                "QR-03 and QR-04 should be captured as review or static-analysis verification items."
            ),
            "severity": "Medium",
            "status": finding_status(non_functional_review_items),
            "reviewed artifact": "unit-test-design.md",
            "requirement impact": "Affects QR-03, QR-04 and DA-03",
            "disposition / action taken": (
                "The unit-test design contains a dedicated review/static-analysis verification section."
                if non_functional_review_items
                else "Add a dedicated review/static-analysis section for QR-03 and QR-04."
            ),
            "evidence": "unit-test-design.md",
            "comments": (
                "Verification remains to be performed when implementation files exist."
                if non_functional_review_items
                else "Action required."
            ),
        }
    )

    return findings


def build_coverage_rows(unit_test_design: str, testcase_rows: list[dict[str, str]]) -> list[tuple[str, str, str]]:
    coverage_rows: list[tuple[str, str, str]] = []

    for requirement_id in REQUIREMENT_IDS:
        testcase_ids = csv_rows_with_requirement(testcase_rows, requirement_id)
        if testcase_ids:
            coverage_rows.append((requirement_id, "Covered", format_testcase_links(testcase_ids)))
            continue

        if requirement_id in {"QR-03", "QR-04"}:
            status = "Review/static-analysis item" if requirement_id in unit_test_design else "Not covered"
            evidence = "unit-test-design.md" if requirement_id in unit_test_design else "No evidence found"
            coverage_rows.append((requirement_id, status, evidence))
            continue

        if requirement_id == "QR-05":
            status = "Covered by test-data constraints" if requirement_id in unit_test_design else "Not covered"
            evidence = "unit-test-design.md" if requirement_id in unit_test_design else "No evidence found"
            coverage_rows.append((requirement_id, status, evidence))
            continue

        coverage_rows.append((requirement_id, "Not covered", "No evidence found"))

    return coverage_rows


def build_review_decisions(findings: list[dict[str, str]]) -> dict[str, object]:
    all_closed = all(item["status"] == "Closed" for item in findings)
    overall_status = "accepted-with-follow-up" if all_closed else "needs-rework"
    overall_summary = (
        "The unit-test design is accepted as aligned with the current MVP requirements baseline. "
        "Review findings raised during the review are closed. Remaining follow-up items are "
        "implementation-time and project-decision items, not blockers for the design baseline."
        if all_closed
        else "The unit-test design still has open review findings and requires updates before full acceptance."
    )

    return {
        "reviewId": f"unit-test-design-review-{date.today().isoformat()}",
        "date": date.today().isoformat(),
        "scope": "Unit test design review for the Diagnostic DID Read MVP",
        "artifactsReviewed": ["unit-test-design.md", "ut-design-testcase.csv"],
        "references": {
            "requirements": "requirements.md",
            "design": "design.md",
            "report": "review-report.md",
            "findings": "review-findings.csv",
        },
        "overallDecision": {
            "status": overall_status,
            "summary": overall_summary,
        },
        "decisions": [
            {
                "id": "DEC-001",
                "topic": "Requirements-first traceability",
                "decision": "Use requirements.md as the primary source of truth and design.md as supporting detail for test design review.",
                "rationale": "This preserves requirement-based verification and stronger audit traceability.",
                "status": "approved" if findings[0]["status"] == "Closed" else "pending",
                "evidence": ["review-findings.csv#RVW-001", "review-report.md"],
            },
            {
                "id": "DEC-002",
                "topic": "DID byte-order verification",
                "decision": "Include an explicit reversed-byte DID scenario to prove high-byte-first DID decoding.",
                "rationale": "FR-01 requires network byte order, and the review requires direct evidence that byte reversal does not accidentally match a supported DID.",
                "status": "approved" if findings[1]["status"] == "Closed" else "pending",
                "evidence": ["review-findings.csv#RVW-002", "ut-design-testcase.csv#UT-024"],
            },
            {
                "id": "DEC-003",
                "topic": "Per-test requirement traceability",
                "decision": "Keep requirement references in each CSV test-case row.",
                "rationale": "This improves review efficiency, traceability, and future verification against DA-03.",
                "status": "approved" if findings[2]["status"] == "Closed" else "pending",
                "evidence": ["review-findings.csv#RVW-003", "ut-design-testcase.csv"],
            },
            {
                "id": "DEC-004",
                "topic": "Boundary-case completeness",
                "decision": "Retain explicit zero-capacity and wrong-SID insufficient-capacity scenarios in the CSV test matrix.",
                "rationale": "These strengthen FR-08 and QR-01 coverage and confirm no-write behavior at important boundaries.",
                "status": "approved" if findings[3]["status"] == "Closed" else "pending",
                "evidence": [
                    "review-findings.csv#RVW-004",
                    "ut-design-testcase.csv#UT-025",
                    "ut-design-testcase.csv#UT-026",
                ],
            },
            {
                "id": "DEC-005",
                "topic": "Non-functional verification method",
                "decision": "Verify QR-03 and QR-04 by implementation review and, where available, static analysis rather than by functional unit tests alone.",
                "rationale": "No dynamic allocation, portable C99 usage, fixed-width types, and named constants are not fully proven by the current functional test design.",
                "status": "approved" if findings[4]["status"] == "Closed" else "pending",
                "evidence": ["review-findings.csv#RVW-005", "unit-test-design.md"],
            },
        ],
        "followUpItems": [
            {
                "id": "FUP-001",
                "item": "Select the unit-test framework or self-contained harness.",
                "status": "open",
                "blocking": False,
            },
            {
                "id": "FUP-002",
                "item": "Select fictional fixed payload bytes for both supported DIDs.",
                "status": "open",
                "blocking": False,
            },
            {
                "id": "FUP-003",
                "item": "Verify QR-03 and QR-04 during source review or static analysis once implementation files exist.",
                "status": "open",
                "blocking": False,
            },
            {
                "id": "FUP-004",
                "item": "Execute the designed tests and record objective results separately.",
                "status": "open",
                "blocking": False,
            },
        ],
    }


def write_findings_csv(path: Path, findings: list[dict[str, str]]) -> None:
    fieldnames = [
        "finding id",
        "title",
        "finding description",
        "severity",
        "status",
        "reviewed artifact",
        "requirement impact",
        "disposition / action taken",
        "evidence",
        "comments",
    ]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, quoting=csv.QUOTE_ALL)
        writer.writeheader()
        writer.writerows(findings)


def write_review_decisions_json(path: Path, review_decisions: dict[str, object]) -> None:
    path.write_text(json.dumps(review_decisions, indent=2) + "\n", encoding="utf-8")


def write_review_report(
    path: Path,
    findings: list[dict[str, str]],
    coverage_rows: list[tuple[str, str, str]],
) -> None:
    all_closed = all(item["status"] == "Closed" for item in findings)
    summary_status = "currently closed" if all_closed else "open items"

    finding_lines = [
        "| ID | Finding | Severity | Status | Disposition |",
        "|---|---|---|---|---|",
    ]
    for item in findings:
        finding_lines.append(
            f"| {item['finding id']} | {item['title']} | {item['severity']} | {item['status']} | {item['disposition / action taken']} |"
        )

    coverage_lines = [
        "| Requirement | Assessment | Evidence |",
        "|---|---|---|",
    ]
    for requirement_id, assessment, evidence in coverage_rows:
        coverage_lines.append(f"| {requirement_id} | {assessment} | {evidence} |")

    report_text = "\n".join(
        [
            "# Review Report: Unit Test Design for Diagnostic DID Read MVP",
            "",
            f"**Date:** {date.today().isoformat()}  ",
            "**Artifact reviewed:** [unit-test-design.md](unit-test-design.md), [ut-design-testcase.csv](ut-design-testcase.csv)  ",
            "**Reference requirements:** [requirements.md](requirements.md)  ",
            "**Reference design:** [design.md](design.md)  ",
            "**Findings register:** [review-findings.csv](review-findings.csv)  ",
            "**Decision record:** [review-decisions.json](review-decisions.json)",
            "",
            "## 1. Review objective",
            "",
            "Review the unit-test design against the documented requirements for the standalone Diagnostic DID Read MVP and record findings, actions taken, and remaining follow-up items.",
            "",
            "## 2. Review scope",
            "",
            "This review covered:",
            "",
            "- Functional requirement coverage for `DcmMvp_ProcessRequest`",
            "- Boundary and negative-test completeness",
            "- Traceability from tests to requirements",
            "- Buffer-safety-oriented test intent",
            "- Non-functional verification needs that cannot be fully proven by functional unit tests alone",
            "",
            "This review did not execute tests or inspect production source code, because implementation files are not yet part of the reviewed scope.",
            "",
            "## 3. Review inputs",
            "",
            "- [requirements.md](requirements.md)",
            "- [design.md](design.md)",
            "- [unit-test-design.md](unit-test-design.md)",
            "- [ut-design-testcase.csv](ut-design-testcase.csv)",
            "- [AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md](AUTOSAR_DCM_GitHub_Copilot_Training_MVP.md)",
            "",
            "## 3.1 Review outputs",
            "",
            "- [review-report.md](review-report.md)",
            "- [review-findings.csv](review-findings.csv)",
            "- [review-decisions.json](review-decisions.json)",
            "",
            "## 4. Review summary",
            "",
            "The unit-test design is reviewed against the documented requirements for the MVP scope. The review checks functional scenario coverage, byte-order verification, capacity-boundary coverage, and requirement traceability.",
            "",
            f"All review findings raised during this review are recorded in [review-findings.csv](review-findings.csv). The current status is: {summary_status}.",
            "The resulting review decisions are recorded in [review-decisions.json](review-decisions.json).",
            "",
            "## 5. Findings and disposition",
            "",
            *finding_lines,
            "",
            "## 6. Coverage assessment against requirements",
            "",
            *coverage_lines,
            "",
            "## 7. Residual notes",
            "",
            "The test design is suitable for requirement-based unit-test implementation, but the following items still depend on later implementation and project decisions:",
            "",
            "- Select the unit-test framework or self-contained harness.",
            "- Select fictional fixed payload bytes for both supported DIDs.",
            "- Verify `QR-03` and `QR-04` during source review or static analysis once implementation files exist.",
            "- Execute the designed tests and record objective results separately.",
            "",
            "## 8. Conclusion",
            "",
            "The review artifacts provide a repeatable requirement-based check of the current unit-test design. Re-run this script after each meaningful change to the reviewed documents so the report and findings register stay synchronized with the latest design state.",
            "",
        ]
    )
    path.write_text(report_text, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Regenerate review-report.md, review-findings.csv, and review-decisions.json from the current design-review artifacts."
    )
    parser.add_argument(
        "--docs-dir",
        type=Path,
        default=DEFAULT_DOCS_DIR,
        help="Directory containing requirements.md, unit-test-design.md, and ut-design-testcase.csv.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    docs_dir = args.docs_dir.resolve()

    requirements_path = docs_dir / "requirements.md"
    unit_test_design_path = docs_dir / "unit-test-design.md"
    testcase_csv_path = docs_dir / "ut-design-testcase.csv"
    review_report_path = docs_dir / "review-report.md"
    review_findings_path = docs_dir / "review-findings.csv"
    review_decisions_path = docs_dir / "review-decisions.json"

    required_paths = [requirements_path, unit_test_design_path, testcase_csv_path]
    missing_paths = [path for path in required_paths if not path.is_file()]
    if missing_paths:
        missing_text = ", ".join(str(path) for path in missing_paths)
        raise FileNotFoundError(f"Required input file(s) not found: {missing_text}")

    _ = extract_requirement_refs(read_text(requirements_path))
    unit_test_design = read_text(unit_test_design_path)
    testcase_rows = read_csv_rows(testcase_csv_path)

    findings = build_findings(unit_test_design, testcase_rows)
    coverage_rows = build_coverage_rows(unit_test_design, testcase_rows)
    review_decisions = build_review_decisions(findings)

    write_findings_csv(review_findings_path, findings)
    write_review_decisions_json(review_decisions_path, review_decisions)
    write_review_report(review_report_path, findings, coverage_rows)

    print(f"Updated: {review_report_path}")
    print(f"Updated: {review_findings_path}")
    print(f"Updated: {review_decisions_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
