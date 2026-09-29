# CI/CD Pipeline Integration with TDD Workflow

## Overview

The newly created GitHub Actions CI/CD pipeline (`ci.yml`) integrates with the existing Test-Driven Development (TDD) infrastructure to create a complete development workflow from requirements through implementation, testing, and deployment.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  Development Workflow                   │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  1. Requirements Definition (requirements.md)           │
│     └─> Functional (FR-01 to FR-09)                    │
│     └─> Quality (QR-01 to QR-05)                       │
│     └─> Delivery (DA-01 to DA-05)                      │
│                                                          │
│  2. Design Artifacts (design.md)                        │
│     └─> Supported DIDs                                 │
│     └─> Request/Response formats                       │
│     └─> Error handling                                 │
│                                                          │
│  3. Test Design (unit-test-design.md, ut-design-testcase.csv)
│     └─> 26 test cases (UT-001 to UT-026)              │
│     └─> Requirements traceability                      │
│     └─> Test types and expected outcomes               │
│     └─> Coverage assessment                            │
│                                                          │
│  4. Review & Quality Gates (review-design.py)          │
│     └─> Generate review-report.md                      │
│     └─> Generate review-findings.csv                   │
│     └─> Generate review-decisions.json                 │
│     └─> Verify design artifacts exist (in CI)          │
│                                                          │
│  5. Implementation (src/dcm_mvp.c)                      │
│     └─> Unit test harness (tests/test_dcm_mvp.c)      │
│     └─> Production code                                │
│                                                          │
│  6. Compile (ci.yml: build-and-test job)              │
│     └─> CMakeLists.txt configuration                   │
│     └─> Multi-compiler matrix (gcc, clang)             │
│     └─> Warnings as errors (-Werror)                   │
│                                                          │
│  7. Unit Testing (ci.yml: build-and-test job)          │
│     └─> CTest execution                                │
│     └─> Test result aggregation                        │
│                                                          │
│  8. Static Analysis (ci.yml: build-and-test job)       │
│     └─> cppcheck (comprehensive linting)               │
│     └─> cpplint (style checking)                       │
│                                                          │
│  9. Design Validation (ci.yml: code-review job)        │
│     └─> Verify test design artifacts exist             │
│     └─> Verify requirement traceability                │
│     └─> Ensure TDD compliance (design before code)     │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

## Workflow Integration Points

### 1. Pre-Implementation Phase (TDD)

**Tools:**
- Custom GitHub Copilot agent (`.github/agents/ut-test-design.agent.md`)
- Test-design skill (`.github/skills/test-design/SKILL.md`)

**Steps:**
1. Developer uses `ut-test-design` agent to generate test artifacts
2. Agent enforces TDD constraint: NO production code generated
3. Output: `unit-test-design.md` and `ut-design-testcase.csv`
4. Developer runs `review-design.py` locally to generate review artifacts
5. Design review is performed (possibly via PR/peer review)
6. Review findings resolved before implementation begins

**CI Check:** `code-review` job verifies these artifacts exist

### 2. Implementation Phase

**What's Required:**
- `src/dcm_mvp.c` - Production code implementation
- `tests/test_dcm_mvp.c` - Unit test harness and test functions
- [Optional] `include/dcm_mvp.h` - Public header file

**Constraints:**
- Must follow design from `unit-test-design.md`
- Must implement all test cases from `ut-design-testcase.csv`
- Must not use dynamic memory allocation (QR-03)
- Must use C99 portable code with named constants (QR-04)
- Must be compilable with gcc and clang

**No CI Automation Yet:**
- Implementation files don't exist in this training MVP
- When implemented, CI will verify they compile and pass tests
- Static analysis will check for compliance with QR-03 and QR-04

### 3. Build Phase (`build-and-test` job)

**Triggered by:**
- Push to main/develop
- Pull request against main/develop
- Manual workflow dispatch

**Steps:**
```
Install dependencies
  ↓
Compile with GCC
  ├─ CMake configuration
  ├─ Build with C99 standard
  ├─ Warnings as errors
  └─ Artifact: binary executable, object files
  ↓
Run unit tests
  ├─ CTest discovery
  ├─ Execute test_dcm_mvp
  └─ Report: pass/fail, coverage
  ↓
Static Analysis (cppcheck)
  ├─ Full enable mode
  ├─ C99 standard check
  └─ Report: cppcheck-report.txt
  ↓
Static Analysis (cpplint)
  ├─ Style validation
  └─ Report: cpplint-report.txt
  ↓
Upload artifacts
  └─ Artifacts: binaries, reports, test results
  ↓
[Repeat with Clang]
```

**Quality Gates:**
- ✅ Compile with 0 errors (both gcc & clang)
- ✅ All unit tests pass
- ✅ No critical static analysis violations
- ❌ Any failure blocks PR merge

### 4. Code Review Phase (`code-review` job)

**Checks:**
1. **Design Artifact Presence**
   - `Docs/unit-test-design.md` must exist (FAIL if missing)
   - `Docs/ut-design-testcase.csv` must exist (FAIL if missing)
   - `Docs/review-report.md` should exist (WARN if missing)

2. **Traceability Verification**
   - CSV contains "Req:" references (WARN if absent)
   - Validates TDD flow (design before implementation)

**Rationale:**
- Prevents accidental skipping of design phase
- Ensures test design documents the implementation
- Validates requirement-to-test traceability

### 5. Summary Phase (`summary` job)

**Aggregates:**
- Build & test results (by compiler)
- Code review results
- Overall pass/fail decision

**Outputs:**
- Consolidated status visible in PR checks
- Detailed logs in GitHub Actions UI
- Downloadable test results and lint reports

## Design-First Enforcement

The CI pipeline actively enforces the TDD constraint:

1. **Skill Definition** (`.github/skills/test-design/SKILL.md`)
   - Strict statement: "DO NOT create production code"
   - Defines what artifacts must be generated
   
2. **Custom Agent** (`.github/agents/ut-test-design.agent.md`)
   - Used to generate design artifacts
   - Configured with limited tool access
   - Explicit TDD constraint in description
   
3. **CI Code Review Job**
   - Verifies design artifacts exist before code can be merged
   - Ensures no code without preceding design
   - Runs on every PR

This creates a multi-layered enforcement:
```
Skill Definition (level 1: documentation)
    ↓
Agent Configuration (level 2: tool defaults)
    ↓
CI Quality Gate (level 3: automated verification)
```

## File Dependencies

```
Docs/requirements.md
    └─> (input to) .github/agents/ut-test-design.agent.md
        └─> (generates) Docs/unit-test-design.md
                    └─> (consumed by) src/dcm_mvp.c (implementation)
                    └─> (consumed by) tests/test_dcm_mvp.c (test harness)
                    └─> (input to) review-design.py
                    └─> (generates) Docs/review-report.md ──┐
                                                             ├─> (checked by) CI code-review job
        └─> (generates) Docs/ut-design-testcase.csv ───────┤
                    └─> (consumed by) tests/test_dcm_mvp.c 
                    └─> (input to) review-design.py
                    └─> (generates) Docs/review-findings.csv ┤
                                                             ├─> (archived by) GitHub Actions
        └─> (generates) review-design.py (automation) ──┐
                    ├─> (generates) Docs/review-decisions.json ┘

CMakeLists.txt
    └─> (input to) CI build-and-test job
        └─> (compiles) src/dcm_mvp.c
        └─> (compiles) tests/test_dcm_mvp.c
        └─> (runs) test_dcm_mvp executable
        └─> (produces) test results
        └─> (analyzed by) cppcheck, cpplint
```

## Configuration for Team

### Before First CI Run

1. **Select Build System**
   - Current: CMake (template provided)
   - Alternatives: Make, Meson, Bazel
   - Update: `CMakeLists.txt` to match selection

2. **Select Test Framework**
   - Suggested: Unity
   - Alternatives: CUnit, self-contained harness
   - Create: `tests/test_dcm_mvp.c` with selected framework

3. **Configure Linting Rules**
   - Current: cppcheck (all checks), cpplint (style)
   - Update: Filter options, ignore patterns as needed
   - Create: `.cppcheckignore` or `cpplint.py` config if necessary

4. **Verify Compiler Targets**
   - Current: GCC, Clang on Ubuntu
   - Consider: Windows runner (windows-latest) for MSVC support
   - Update: `ci.yml` if other compilers needed

### During Development

1. **Test Design Phase**
   ```powershell
   # Generate test design artifacts using custom agent
   # Verify locally with:
   python .\review-design.py
   ```

2. **Implementation Phase**
   ```powershell
   # Create src/dcm_mvp.c and tests/test_dcm_mvp.c
   # Build locally:
   mkdir build ; cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   cmake --build . --config Debug
   
   # Test locally:
   ctest --output-on-failure
   ```

3. **PR Phase**
   - Push to develop branch
   - GitHub Actions automatically runs full CI pipeline
   - Review results in PR checks section
   - Address any failures before merge

### After Merge

1. **Artifact Review**
   - Download test results and lint reports from Actions
   - Verify all checks passed on main branch
   - Archive reports for compliance/traceability

2. **Release Preparation**
   - Tag release on main branch
   - Consider adding release workflow to `ci.yml`
   - Package and document

## Continuous Improvement

**Follow-up Items (from review-decisions.json):**

- **FUP-001**: Select unit-test framework
  - Action: Team decides between Unity, CUnit, custom harness
  - Update: CMakeLists.txt, ci.yml with test runner
  - Impact: Enables automated test execution in CI

- **FUP-002**: Select DID payload values
  - Action: Team chooses fictional VIN and DID values
  - Update: Docs/unit-test-design.md test data, test_dcm_mvp.c
  - Impact: Makes tests concrete and executable

- **FUP-003**: Verify QR-03/QR-04 via review/static-analysis
  - Action: Team reviews implementation for no dynamic allocation, named constants
  - Update: CI linting configuration, code review checklist
  - Impact: Automates quality gate for non-functional requirements

- **FUP-004**: Execute tests and record results
  - Action: Run complete test suite after implementation
  - Update: Test results in GitHub Actions artifacts
  - Impact: Validates design-to-implementation traceability

## Testing the CI Pipeline Locally

Without full implementation, you can verify the workflow structure:

```powershell
# 1. Verify workflow syntax
# (GitHub Actions provides syntax validation on PR)

# 2. Simulate design phase
python .\review-design.py
# Check: Docs/review-report.md, review-findings.csv, review-decisions.json generated

# 3. Verify design artifacts exist (code-review job check)
if (Test-Path "Docs\unit-test-design.md" -and 
    Test-Path "Docs\ut-design-testcase.csv") {
    Write-Host "Design artifacts present - CI code-review will pass"
} else {
    Write-Host "Design artifacts missing - CI code-review will fail"
}

# 4. When implementation is ready, test locally:
mkdir build ; cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest --output-on-failure
```

## Conclusion

The GitHub Actions CI/CD pipeline enforces and validates the TDD approach by:
1. **Requiring design artifacts** before code can be merged
2. **Compiling across multiple compilers** for portability
3. **Running all unit tests** before release
4. **Performing static analysis** for code quality
5. **Archiving results** for compliance and traceability

This creates a complete feedback loop from requirements through design, testing, implementation, validation, and release.
