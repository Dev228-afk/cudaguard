# CudaGuard v1.0.0 — CUDA C++ Static Analysis + Driver Diagnostics

## Highlights

- **5 CUDA-focused AST analysis rules** — host/device qualifier misuse, kernel launch validation, error check enforcement, shared memory verification, and memcpy direction checking
- **Clang LibTooling + AST Matchers** — semantic analysis, not regex
- **`compile_commands.json` support** — integrates with real build systems
- **`nvcc` wrapper mode** — pre-compilation diagnostic layer with measured <5% overhead
- **Human-readable and JSON diagnostics** — file/line/column, rule IDs, actionable fix hints
- **40+ regression test cases** — positive, negative, and edge cases across all rules
- **Benchmark script** — reproducible performance measurement

## Supported Rules

| Rule | Description |
| ---- | ----------- |
| CG001 | Kernel launch not followed by CUDA error check |
| CG002 | Suspicious kernel launch dimensions (zero, oversized) |
| CG003 | `__device__` function calls host-only function |
| CG004 | Kernel uses `extern __shared__` but launch omits size |
| CG005 | `cudaMemcpy` direction mismatch (heuristic) |

## Usage

```bash
# Build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# Analyze
./build/cudaguard --file kernel.cu -- -x cuda --cuda-gpu-arch=sm_75

# Wrapper mode
./build/cudaguard --wrap-nvcc -- nvcc -arch=sm_75 kernel.cu -o kernel

# JSON output
./build/cudaguard --json --file kernel.cu -- -x cuda --cuda-gpu-arch=sm_75
```

## Requirements

- Linux (x86_64)
- C++20 compiler (GCC 12+ or Clang 15+)
- LLVM/Clang development libraries (14+)
- CMake 3.20+
- Python 3 (for regression tests and benchmarking)
- CUDA Toolkit (optional — for wrapper mode and full header resolution)

## What This Is

A Clang LibTooling-based static analysis and build-diagnostics tool for CUDA C++.

## What This Is Not

A CUDA compiler, PTX generator, or replacement for nvcc.
