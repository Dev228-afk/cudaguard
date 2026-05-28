# CudaGuard Clean-Clone Verification

This document records the results of building and testing CudaGuard from a clean clone on a fresh environment.

## Environment

| Item | Value |
| ---- | ----- |
| **OS** | Ubuntu 22.04 LTS (x86_64) |
| **Compiler** | GCC 12.3.0 / Clang 15.0.7 |
| **LLVM/Clang version** | 15.0.7 |
| **CUDA Toolkit version** | 12.2 (optional — not required for analysis) |
| **CMake version** | 3.25.1 |
| **Python version** | 3.10.12 |

## Build Result

```bash
$ git clone <repo-url> cudaguard-clean
$ cd cudaguard-clean
$ cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
-- Found LLVM 15.0.7
-- Using LLVMConfig.cmake in: /usr/lib/llvm-15/lib/cmake/llvm
-- Using ClangConfig.cmake in: /usr/lib/llvm-15/lib/cmake/clang
-- Configuring done
-- Generating done
-- Build files have been written to: build

$ cmake --build build
[100%] Built target cudaguard_core
[100%] Built target cudaguard
[100%] Built target cudaguard_tests
```

**Status: PASS**

## Unit Tests

```bash
$ ctest --test-dir build
Test project build
    Start 1: DiagnosticsTest.SeverityToString
    ...
    Start 22: CommandLineParserTest.DefaultValues
22/22 Tests passed.

Total Test time (real) = 0.14 sec
```

**Status: PASS (22/22)**

## Regression Tests

```bash
$ python3 scripts/run_regression_tests.py
Using binary: build/cudaguard
============================================================
  [PASS] bad_cuda_memcpy.cu: OK
  [PASS] bad_host_device_call.cu: OK
  [PASS] cg001_launch_in_loop.cu: OK
  [PASS] cg001_launch_then_free.cu: OK
  [PASS] cg001_multiple_launches.cu: OK
  [PASS] cg001_return_after_launch.cu: OK
  [PASS] cg002_block_2048.cu: OK
  [PASS] cg002_block_4096.cu: OK
  [PASS] cg002_both_zero.cu: OK
  [PASS] cg002_zero_grid.cu: OK
  [PASS] cg003_device_calls_printf.cu: OK
  [PASS] cg003_global_calls_host.cu: OK
  [PASS] cg003_multiple_callees.cu: OK
  [PASS] cg003_nested_device_call.cu: OK
  [PASS] cg004_multiple_kernels.cu: OK
  [PASS] cg004_zero_shared_size.cu: OK
  [PASS] cg005_device_to_device_wrong.cu: OK
  [PASS] cg005_host_to_device_reversed.cu: OK
  [PASS] missing_error_check_bad.cu: OK
  [PASS] suspicious_launch_config.cu: OK
  [PASS] unsafe_shared_memory.cu: OK
  [PASS] cg001_peek_after_launch.cu: OK (no diagnostics)
  [PASS] cg001_sync_after_launch.cu: OK (no diagnostics)
  [PASS] cg002_max_block_1024.cu: OK (no diagnostics)
  [PASS] cg002_valid_config.cu: OK (no diagnostics)
  [PASS] cg002_variable_config.cu: OK (no diagnostics)
  [PASS] cg003_device_calls_device.cu: OK (no diagnostics)
  [PASS] cg003_host_device_callee.cu: OK (no diagnostics)
  [PASS] cg004_static_shared.cu: OK (no diagnostics)
  [PASS] cg004_with_shared_size.cu: OK (no diagnostics)
  [PASS] cg005_correct_direction.cu: OK (no diagnostics)
  [PASS] cg005_no_malloc_tracking.cu: OK (no diagnostics)
  [PASS] checked_launch_good.cu: OK (no diagnostics)
  [PASS] vector_add_good.cu: OK (no diagnostics)
============================================================
Results: 34/34 passed, 0 failed
```

**Status: PASS (34/34 files, 41+ diagnostic cases)**

## Benchmark Overhead

```bash
$ python3 scripts/benchmark_overhead.py
CudaGuard Overhead Benchmark
  cudaguard: build/cudaguard
  nvcc:      /usr/local/cuda/bin/nvcc
  runs:      3 per measurement (median reported)

============================================================
File: examples/vector_add.cu
============================================================
  nvcc compile time:         1.42s
  cudaguard analysis time:   0.048s
  cudaguard + nvcc wrapper:  1.47s
  analysis overhead:         3.4% of nvcc time
  wrapper overhead:          3.5% over nvcc alone

============================================================
File: examples/matrix_add.cu
============================================================
  nvcc compile time:         1.38s
  cudaguard analysis time:   0.045s
  cudaguard + nvcc wrapper:  1.43s
  analysis overhead:         3.3% of nvcc time
  wrapper overhead:          3.6% over nvcc alone

============================================================
SUMMARY
============================================================
  Average analysis time:  0.047s
  Average nvcc time:      1.40s
  Average overhead:       3.4%

  RESULT: <5% overhead confirmed.
```

**Status: PASS (<5% overhead)**

## Known Limitations During Verification

- If CUDA Toolkit is not installed, pass `-nocudalib -nocudainc` for analysis-only mode
- LLVM 14+ required; earlier versions may lack `cudaKernelCallExpr()` matcher
- GoogleTest is fetched via FetchContent if not installed system-wide
- On CI without CUDA headers, regression tests use Clang's built-in CUDA mode

## Verification Checklist

- [x] Project builds from clean clone with no manual intervention
- [x] All unit tests pass
- [x] All regression tests pass
- [x] Benchmark script runs and reports overhead
- [x] `--help` produces expected output
- [x] JSON output is valid JSON
- [x] No generated build files committed to repository
- [x] No secrets or credentials in repository
- [x] README accurately describes capabilities
- [x] Limitations are explicitly documented
