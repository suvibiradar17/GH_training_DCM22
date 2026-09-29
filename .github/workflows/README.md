# GitHub Actions CI/CD Pipeline

## Overview

This folder contains the GitHub Actions workflow configuration for the AUTOSAR DCM DID Read MVP project.

## Workflow: `ci.yml`

The continuous integration pipeline is defined in `.github/workflows/ci.yml` and runs automatically on:
- **Push**: to `main` or `develop` branches
- **Pull Request**: against `main` or `develop` branches
- **Manual Dispatch**: via GitHub UI

### Pipeline Jobs

#### 1. Build and Test (`build-and-test`)

**Purpose:** Compile code and run unit tests across multiple compilers.

**Runs on:** Ubuntu latest (matrix: gcc, clang)

**Steps:**
1. **Install dependencies** - Build tools and selected compiler
2. **Compile** - Uses CMake to configure and build with C99 standard
3. **Run unit tests** - Executes test suite via CTest
4. **Run linting** - Performs static code analysis (cppcheck, cpplint)
5. **Upload artifacts** - Saves build outputs and lint reports

**Configuration Notes:**
- The workflow uses CMake for build configuration. If your team selects a different build system (Make, MSBuild, etc.), update the compile step to use your chosen tool.
- The workflow is flexible: `continue-on-error: true` allows partial failures to not block the pipeline if some tools aren't installed/configured yet.
- Static analysis tools (cppcheck, cpplint) are installed via `apt-get` and `pip`. Adjust if using platform-specific or commercial tools.

#### 2. Code Review (`code-review`)

**Purpose:** Validate that required design artifacts exist and are properly maintained.

**Checks:**
- Verifies presence of `Docs/unit-test-design.md` (required)
- Verifies presence of `Docs/ut-design-testcase.csv` (required)
- Warns if `Docs/review-report.md` is missing (should be regenerated via review-design.py)
- Checks for requirement traceability in CSV

**TDD Compliance:** Ensures design artifacts exist before implementation is committed.

#### 3. Summary (`summary`)

**Purpose:** Aggregate results and report overall status.

**Outcomes:**
- Summarizes results from build/test and code review jobs
- Reports failure if either job fails
- Provides consolidated status for PR decisions

## Build System and Testing Framework

### Current State

Per the project requirements (**copilot-instructions.md, Section 5**):
> Build system, compiler, compiler version, and CI tool versions: **Not yet selected. Do not assume these.**
> Test library: **Unity is a suggestion, not a confirmed dependency**. Confirm the team's choice before relying on it.

### Adapting the Workflow

When your team selects build and test tools:

1. **Update CMakeLists.txt** - Replace or supplement with your chosen build system
2. **Update compile step** - Modify the compile commands to match your build system
3. **Update test step** - Adjust to match your test runner (Unity, CUnit, custom harness, etc.)
4. **Update linting step** - Select appropriate static analysis tools
5. **Test locally** - Verify workflow runs successfully in your CI environment

### Recommended Selections

**Build System Options:**
- **CMake (current)** - Platform-agnostic, widely used for C projects
- **GNU Make** - Simple for small projects
- **Meson** - Modern build system for C/C++
- **Bazel** - For larger projects with complex dependencies

**Test Framework Options:**
- **Unity** - Lightweight, suggested in training brief
- **CUnit** - Similar to Unity, good for C99
- **Custom Harness** - Simple self-contained test runner (as mentioned in copilot-instructions.md)
- **Catch2** - More feature-rich (C++)

**Linting/Analysis Tools:**
- **cppcheck** - Included in current workflow
- **clang-tidy** - Integrates well with CMake
- **MISRA C checking** - For safety-critical standards
- **Static analysis via compiler** - GCC/Clang built-in warnings

## Artifacts and Reports

The workflow generates and uploads:

- **Build artifacts** (per compiler):
  - Compiled binaries and object files
  - Lint reports (cppcheck-report.txt, cpplint-report.txt)
  
- **Test results** (per compiler):
  - CTest output and XML results
  - Exit codes indicating pass/fail

These are stored as GitHub Actions artifacts and can be downloaded from the PR or workflow run page.

## Local Testing

To verify the workflow logic locally before committing:

```powershell
# Install CMake (if not already installed)
choco install cmake  # on Windows with Chocolatey
# or sudo apt-get install cmake on Linux

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build . --config Debug

# Run tests (if test framework is set up)
ctest --output-on-failure

# Run linting (if cppcheck is available)
cppcheck --enable=all --std=c99 ../src ../include
```

## Customization Checklist

- [ ] Build system selected (CMake / Make / other)
- [ ] C99 compiler configured (GCC / Clang / MSVC)
- [ ] Test framework selected (Unity / CUnit / self-contained)
- [ ] CMakeLists.txt updated to match build system
- [ ] Compiler flags verified (warnings, optimizations)
- [ ] Test discovery configured (CTest / other runner)
- [ ] Linting tool configured
- [ ] Workflow tested locally
- [ ] Dependencies documented in README.md
- [ ] CI status badge added to README.md

## Status Reporting

After each workflow run:
- GitHub will display status in the PR checks section
- Detailed logs available via "Details" link
- Artifacts available in "Summary" tab of workflow run

## Troubleshooting

**Build fails but locally it works:**
- Ensure Ubuntu environment matches your local setup (compiler versions, stdlib paths)
- Consider adding a Windows runner (windows-latest) to CI matrix if targeting Windows

**Tests don't run:**
- Verify test framework is installed/linked in CMakeLists.txt
- Check CTest discovery of test executables

**Linting produces false positives:**
- Adjust cppcheck/cpplint filter options
- Consider adding .cppcheckignore or cpplint config files

**Artifacts not uploading:**
- Check that build output directory paths match upload paths
- Verify artifact names don't conflict

## Next Steps

1. **Confirm build system and test framework** (resolves FUP-001 partial)
2. **Configure DID test payload values** (FUP-002)
3. **Implement test harness** with selected test framework
4. **Implement production code** (DCM module)
5. **Execute tests in CI** and verify green status
