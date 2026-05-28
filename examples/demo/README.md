# CudaGuard Demo Examples

Each file demonstrates exactly one CudaGuard diagnostic rule in isolation.

## Files

| File | Rule | What It Shows |
|------|------|---------------|
| `good_vector_add.cu` | (none) | Clean CUDA code that passes all checks |
| `bad_missing_error_check.cu` | CG001 | Kernel launch without `cudaGetLastError()` |
| `bad_host_device_call.cu` | CG003 | `__device__` function calling host-only function |
| `bad_shared_memory.cu` | CG004 | `extern __shared__` without launch-time size |
| `bad_memcpy_direction.cu` | CG005 | `cudaMemcpy` direction inconsistent with pointer provenance |

## Quick Demo

```bash
# Clean code — no diagnostics
./build/cudaguard --file examples/demo/good_vector_add.cu -- -x cuda --cuda-gpu-arch=sm_75

# CG001: Missing error check
./build/cudaguard --file examples/demo/bad_missing_error_check.cu -- -x cuda --cuda-gpu-arch=sm_75

# CG003: Host/device qualifier misuse
./build/cudaguard --file examples/demo/bad_host_device_call.cu -- -x cuda --cuda-gpu-arch=sm_75

# CG004: Shared memory launch mismatch
./build/cudaguard --file examples/demo/bad_shared_memory.cu -- -x cuda --cuda-gpu-arch=sm_75

# CG005: Memcpy direction mismatch
./build/cudaguard --file examples/demo/bad_memcpy_direction.cu -- -x cuda --cuda-gpu-arch=sm_75

# JSON output
./build/cudaguard --json --file examples/demo/bad_host_device_call.cu -- -x cuda --cuda-gpu-arch=sm_75
```

## Expected Outputs

**CG001:**
```
bad_missing_error_check.cu:25:5: warning: CG001: kernel launch is not followed by cudaGetLastError, cudaPeekAtLastError, or cudaDeviceSynchronize
  hint: add cudaGetLastError() after the launch to catch asynchronous launch failures
```

**CG003:**
```
bad_host_device_call.cu:17:5: error: CG003: __device__ function 'deviceCompute' calls function 'logValue' that is not marked __device__ or __host__ __device__
  hint: add __device__ or __host__ __device__ qualifier to 'logValue'
```

**CG004:**
```
bad_shared_memory.cu:36:5: warning: CG004: kernel 'histogram' uses extern __shared__ memory, but launch does not provide a dynamic shared-memory size
  hint: use the third kernel launch parameter to specify dynamic shared memory size
```

**CG005:**
```
bad_memcpy_direction.cu:20:5: warning: CG005: cudaMemcpy direction may not match known pointer categories
  hint: destination 'd_data' was allocated with cudaMalloc (device), but copy direction is DeviceToHost
```
