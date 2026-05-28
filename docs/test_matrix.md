# CudaGuard Test Matrix

**Total: 41+ regression test cases across 5 rules**

## Test Case Breakdown by Rule

| Rule | Positive Cases | Negative Cases | Edge Cases | Total |
|------|---------------|----------------|------------|-------|
| CG001 | 5 | 3 | 2 | 10 |
| CG002 | 5 | 3 | 2 | 10 |
| CG003 | 5 | 2 | 2 | 9 |
| CG004 | 3 | 2 | 2 | 7 |
| CG005 | 3 | 2 | 1 | 6 |
| **Total** | **21** | **12** | **9** | **42** |

## CG001: Missing CUDA Error Check (10 cases)

### Positive Cases (should trigger warning)
| File | Description |
|------|-------------|
| `missing_error_check_bad.cu` | Single launch, no error check |
| `cg001_multiple_launches.cu` | Two launches, both unchecked |
| `cg001_launch_in_loop.cu` | Launch inside loop body, unchecked |
| `cg001_launch_then_free.cu` | Launch followed only by cudaFree |
| `cg001_return_after_launch.cu` | Launch followed by immediate return |

### Negative Cases (should NOT trigger warning)
| File | Description |
|------|-------------|
| `vector_add_good.cu` | cudaGetLastError immediately after launch |
| `cg001_sync_after_launch.cu` | cudaDeviceSynchronize after launch |
| `cg001_peek_after_launch.cu` | cudaPeekAtLastError after launch |

### Edge Cases
| File | Description |
|------|-------------|
| `checked_launch_good.cu` | Error check with early return on failure |
| `cg001_launch_in_loop.cu` | Compound statement boundary (loop) |

## CG002: Suspicious Kernel Launch Configuration (10 cases)

### Positive Cases (should trigger warning)
| File | Description |
|------|-------------|
| `suspicious_launch_config.cu` | Zero block + block > 1024 in one file |
| `cg002_zero_grid.cu` | Grid dimension is literal 0 |
| `cg002_block_2048.cu` | Block size 2048 (over max) |
| `cg002_block_4096.cu` | Block size 4096 (extremely over max) |
| `cg002_both_zero.cu` | Both grid and block are 0 |

### Negative Cases (should NOT trigger warning)
| File | Description |
|------|-------------|
| `cg002_valid_config.cu` | Normal 8 blocks x 256 threads |
| `cg002_max_block_1024.cu` | Block = 1024 (max valid) |
| `cg002_variable_config.cu` | Config uses variables, not literals |

### Edge Cases
| File | Description |
|------|-------------|
| `cg002_max_block_1024.cu` | Boundary: exactly 1024 (valid) |
| `cg002_both_zero.cu` | Multiple issues in one launch |

## CG003: Host/Device Qualifier Misuse (9 cases)

### Positive Cases (should trigger error)
| File | Description |
|------|-------------|
| `bad_host_device_call.cu` | __device__ calls two host functions |
| `cg003_multiple_callees.cu` | __device__ calls three host functions |
| `cg003_global_calls_host.cu` | __global__ kernel calls host function |
| `cg003_nested_device_call.cu` | Inner __device__ func calls host |
| `cg003_device_calls_printf.cu` | __device__ calls host I/O wrapper |

### Negative Cases (should NOT trigger error)
| File | Description |
|------|-------------|
| `cg003_device_calls_device.cu` | __device__ calls __device__ (valid) |
| `cg003_host_device_callee.cu` | __device__ calls __host__ __device__ (valid) |

### Edge Cases
| File | Description |
|------|-------------|
| `cg003_nested_device_call.cu` | Only inner function is flagged, not outer |
| `cg003_global_calls_host.cu` | __global__ treated same as __device__ |

## CG004: Unsafe Dynamic Shared Memory (7 cases)

### Positive Cases (should trigger warning)
| File | Description |
|------|-------------|
| `unsafe_shared_memory.cu` | extern __shared__, no third param |
| `cg004_multiple_kernels.cu` | Two kernels, both missing shared size |
| `cg004_zero_shared_size.cu` | Third param is literal 0 |

### Negative Cases (should NOT trigger warning)
| File | Description |
|------|-------------|
| `cg004_with_shared_size.cu` | Correct: third param provided |
| `cg004_static_shared.cu` | Static shared (not extern) |

### Edge Cases
| File | Description |
|------|-------------|
| `cg004_zero_shared_size.cu` | Explicit 0 same as omitted |
| `cg004_multiple_kernels.cu` | Both kernels independently flagged |

## CG005: cudaMemcpy Direction Mismatch (6 cases)

### Positive Cases (should trigger warning)
| File | Description |
|------|-------------|
| `bad_cuda_memcpy.cu` | Device dst + DeviceToHost direction |
| `cg005_host_to_device_reversed.cu` | Device src + HostToDevice direction |
| `cg005_device_to_device_wrong.cu` | Host dst + DeviceToDevice direction |

### Negative Cases (should NOT trigger warning)
| File | Description |
|------|-------------|
| `cg005_correct_direction.cu` | Correct H2D and D2H directions |
| `cg005_no_malloc_tracking.cu` | Untracked pointers — no false positive |

### Edge Cases
| File | Description |
|------|-------------|
| `cg005_no_malloc_tracking.cu` | Conservative: no warning when unsure |

## Unit Tests (additional)

| File | Test Count | Coverage |
|------|-----------|----------|
| `DiagnosticsTest.cpp` | 7 tests | Formatting, counts, JSON, severity |
| `RuleRegistryTest.cpp` | 4 tests | Registration, enable/disable |
| `CommandLineParserTest.cpp` | 11 tests | All CLI flags and modes |

**Total unit test assertions: 22+**

## Running Tests

```bash
# Unit tests
ctest --test-dir build

# Regression tests
python3 scripts/run_regression_tests.py

# All tests
cmake --build build && ctest --test-dir build && python3 scripts/run_regression_tests.py
```
