# CudaGuard — Interview Preparation Notes

## 30-Second Elevator Pitch

"CudaGuard is a C++20 static analysis and compiler-driver diagnostics tool for CUDA C++. It uses Clang LibTooling and AST Matchers to parse CUDA source files, inspect the AST, and report compiler-style diagnostics for issues like host/device qualifier misuse, suspicious kernel launch configs, missing CUDA error checks, unsafe dynamic shared memory usage, and cudaMemcpy direction mismatches. I also added a lightweight wrapper around compile_commands.json and nvcc so it fits into a normal Linux build flow."

---

## What I Built

A Linux command-line tool that:
- Parses CUDA C++ using Clang LibTooling (not regex or string matching)
- Implements 5 AST-based analysis rules for CUDA-specific mistakes
- Reports diagnostics in compiler-style format (file:line:col: severity: rule: message)
- Supports JSON output for IDE/CI integration
- Loads `compile_commands.json` for real build-system integration
- Wraps `nvcc` to add pre-compilation diagnostics with <5% overhead
- Includes 40+ regression test cases and automated benchmarking

## Why I Built It

I wanted to demonstrate:
1. **C++20 systems programming** — modern language features in a production-style codebase
2. **Compiler front-end tooling** — practical use of Clang's LibTooling and AST infrastructure
3. **CUDA language semantics** — understanding host/device execution spaces, kernel launches, shared memory
4. **Diagnostics design** — structured, actionable output with source locations and fix hints
5. **Build-system integration** — compilation databases, compiler-driver wrapping, artifact management
6. **Testing rigor** — unit tests, regression tests, performance measurement, documented limitations

I scoped it deliberately: it is a **static analysis tool**, not a compiler. It does not generate PTX, LLVM IR, or device code. This is an honest, demonstrable scope.

## Architecture

```
Source Files → Clang Parser → AST → AST Matchers → Rule Callbacks → Diagnostics
```

**Key components:**
- `ClangTool` + `MatchFinder` drive the analysis
- Each rule registers declarative AST matchers
- Rule callbacks receive match results and emit structured `Diagnostic` objects
- The `DiagnosticReporter` formats output (human-readable or JSON)
- The `RuleRegistry` manages rule lifecycle and enable/disable filtering
- The `BuildWrapper` handles nvcc invocation and artifact logging

**Why AST Matchers instead of RecursiveASTVisitor:**
- Declarative, composable pattern matching
- Each rule is independently testable
- Matchers map naturally to the patterns we're detecting (e.g., "find all kernel calls inside compound statements")
- MatchFinder handles traversal — rules don't need to implement visitation logic

## Most Difficult Rule: CG003 (Host/Device Qualifier Misuse)

**What it does:** Detects when a `__device__` or `__global__` function calls a function that lacks appropriate CUDA device qualifiers.

**Why it's hard:**
1. **Callee resolution** — must resolve direct callees through the AST, handling overloads and implicit conversions
2. **Attribute checking** — must distinguish `__device__`, `__host__`, `__host__ __device__`, and `__global__` functions
3. **False positive avoidance:**
   - Skip compiler builtins (`__builtin_*`)
   - Skip implicit functions (compiler-generated constructors, etc.)
   - Skip dependent template contexts (unresolved until instantiation)
   - Skip CUDA intrinsics (prefixed with `__`)
4. **Semantic nuance** — `__global__` functions are callable from host but execute on device, so they should be checked the same way as `__device__` for their internal calls

**Implementation approach:**
```cpp
finder.addMatcher(
    functionDecl(
        anyOf(hasAttr(attr::CUDADevice), hasAttr(attr::CUDAGlobal)),
        forEachDescendant(callExpr().bind("call")))
        .bind("deviceFunc"),
    &callback_);
```

Then in the callback: resolve direct callee → check `hasAttr<CUDADeviceAttr>()` → filter builtins/implicit → emit diagnostic.

**What I'd say in an interview:** "The challenge was making it useful without being noisy. A naive implementation flags everything, including compiler builtins and unresolved template calls. The interesting engineering was deciding what to skip — that's where you need to understand Clang's AST representation of CUDA semantics."

## CG004: Two-Phase Correlation

**What makes it interesting:** Requires correlating information between kernel *definitions* (which declare `extern __shared__`) and kernel *launch sites* (which may omit the shared memory argument).

**Implementation:** Two matchers in a single pass:
1. Matcher 1 collects kernel names that contain `extern __shared__` declarations
2. Matcher 2 inspects kernel launches and cross-references the collected set

This works because Clang processes definitions before call sites in a translation unit. It's a lightweight form of metadata-driven analysis without building full IR.

## Testing Strategy

**Three layers:**
1. **Unit tests (GoogleTest)** — test pure logic: diagnostic formatting, rule registry, CLI parsing
2. **Regression tests (Python runner)** — test actual `.cu` files against expected diagnostic output
3. **Benchmark script** — prove <5% overhead claim with reproducible measurements

**Test coverage:**
- 42 test cases: 21 positive (trigger diagnostic) + 12 negative (verify no false positive) + 9 edge cases
- Each rule has both "should warn" and "should NOT warn" cases
- Edge cases test boundaries (e.g., block size exactly 1024 = valid; 1025 = warn)

## Performance Measurement

- `scripts/benchmark_overhead.py` measures median of 3 runs
- Reports absolute analysis time + percentage overhead vs. nvcc
- Typical result: ~50ms analysis per file, <5% of nvcc compile time
- Wrapper mode adds one process spawn + CudaGuard analysis before invoking nvcc

## Limitations I Document Explicitly

- No interprocedural analysis (by design)
- Template-heavy code may produce incomplete diagnostics
- CG005 (memcpy direction) is heuristic — only warns on high-confidence cases
- Macros may degrade source-location quality
- Only analyzes within a single translation unit

**Why I document these:** Mature tooling acknowledges scope. Interviewers respect "I chose not to" over "I didn't think of that."

## What I Would Improve Next

1. **SARIF output** — standard format for static analysis results (IDE integration)
2. **Fixit hints** — machine-applicable fix suggestions (Clang `FixItHint`)
3. **Cross-TU analysis** — use Clang's Cross-Translation-Unit framework for CG004
4. **Custom CUDA error-check macros** — allow user-defined functions that satisfy CG001
5. **dim3 constructor analysis** — extend CG002 to check `dim3(0, 256, 1)` patterns
6. **clang-tidy integration** — package rules as a clang-tidy module

## Questions I'm Prepared For

**"Why not just use nvcc's own warnings?"**
nvcc catches compilation errors but not semantic patterns like "you forgot to check the error." CudaGuard catches higher-level mistakes before compilation even runs.

**"How does this differ from clang-tidy?"**
clang-tidy doesn't have CUDA-specific checks for kernel launch patterns, shared memory correlation, or memcpy direction analysis. CudaGuard fills that gap.

**"Could this scale to large codebases?"**
Yes — it uses compilation databases (same mechanism as clang-tidy and clangd), processes files independently, and analysis is O(AST size) per file. The wrapper mode integrates into existing build flows without restructuring.

**"What was the hardest debugging session?"**
Getting the CG001 sibling-statement detection right. The Clang AST wraps kernel call expressions in `ExprWithCleanups` and implicit casts, so comparing AST node pointers directly fails. I had to add source-location comparison as a fallback.
