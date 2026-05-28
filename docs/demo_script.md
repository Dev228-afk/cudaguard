# CudaGuard Demo Script

This demo takes under 2 minutes and demonstrates all key capabilities.

## Prerequisites

```bash
# Build the project
git clone <repo-url>
cd cudaguard
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

## Demo Steps

### 1. Show Help (5 seconds)
```bash
./build/cudaguard --help
```

**Talking point:** "CudaGuard supports single-file analysis, compilation database, rule selection, and nvcc wrapper mode."

### 2. Detect Missing Error Check — CG001 (15 seconds)
```bash
./build/cudaguard --file examples/broken_examples/missing_error_check.cu -- -x cuda --cuda-gpu-arch=sm_75
```

**Expected output:**
```
missing_error_check.cu:23:5: warning: CG001: kernel launch is not followed by cudaGetLastError, cudaPeekAtLastError, or cudaDeviceSynchronize
  hint: add cudaGetLastError() after the launch to catch asynchronous launch failures
```

**Talking point:** "This is the most practical rule — kernel launches are async, and forgetting the error check is the most common CUDA bug."

### 3. Detect Suspicious Launch Config — CG002 (15 seconds)
```bash
./build/cudaguard --file examples/broken_examples/wrong_grid_config.cu -- -x cuda --cuda-gpu-arch=sm_75
```

**Talking point:** "The tool only warns on literal zero dimensions or block sizes above 1024 — we're conservative to avoid false positives."

### 4. Detect Host/Device Qualifier Misuse — CG003 (20 seconds)
```bash
./build/cudaguard --file examples/broken_examples/bad_qualifier_usage.cu -- -x cuda --cuda-gpu-arch=sm_75
```

**Talking point:** "This is the most technically interesting rule. It traverses the AST for __device__ functions, resolves callees, and checks for proper CUDA qualifiers. This is real compiler front-end work."

### 5. JSON Output (10 seconds)
```bash
./build/cudaguard --json --file examples/broken_examples/wrong_grid_config.cu -- -x cuda --cuda-gpu-arch=sm_75
```

**Talking point:** "JSON output enables integration with IDEs, CI pipelines, and code review tools."

### 6. Clean File — No Diagnostics (10 seconds)
```bash
./build/cudaguard --file examples/vector_add.cu -- -x cuda --cuda-gpu-arch=sm_75
```

**Expected:**
```
CudaGuard analyzed 1 file(s)
0 error(s), 0 warning(s)
```

**Talking point:** "Well-written CUDA code passes cleanly."

### 7. Regression Tests (15 seconds)
```bash
python3 scripts/run_regression_tests.py
```

**Talking point:** "Automated regression testing ensures rules don't regress as the codebase evolves."

## Key Talking Points

1. "I built this to learn compiler front-end tooling in a CUDA-specific context."
2. "The tool uses Clang AST matchers rather than regex — it understands the semantic structure of CUDA C++."
3. "It reports compiler-style diagnostics with file, line, column, rule ID, and actionable hints."
4. "I intentionally scoped it as a static-analysis/build-diagnostics tool, not a compiler."
5. "The most interesting part was handling CUDA-specific constructs like kernel launches, host/device function attributes, and the two-pass shared memory correlation."
6. "I document limitations explicitly because that's what mature tooling does."

## Wrapper Mode Demo (Optional, requires nvcc)

```bash
./build/cudaguard --wrap-nvcc -- nvcc -arch=sm_75 examples/vector_add.cu -o vector_add
```

**Talking point:** "Wrapper mode integrates into existing build workflows — it runs analysis first, then invokes the real compiler if no errors are found."
