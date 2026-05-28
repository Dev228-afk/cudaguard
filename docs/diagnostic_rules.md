# CudaGuard Diagnostic Rules

## CG001: Missing CUDA Error Check After Kernel Launch

**Severity:** Warning

**Description:** Detects CUDA kernel launches that are not followed by a call to `cudaGetLastError()`, `cudaPeekAtLastError()`, or `cudaDeviceSynchronize()` within the next 5 statements.

**Rationale:** CUDA kernel launches are asynchronous. If a launch fails, the only way to detect it is by checking the CUDA error state. Omitting this check leads to silent failures that are difficult to debug.

**Bad Example:**
```cuda
vectorAdd<<<blocks, threads>>>(d_a, d_b, d_c, n);
cudaFree(d_a);  // No error check!
return 0;
```

**Good Example:**
```cuda
vectorAdd<<<blocks, threads>>>(d_a, d_b, d_c, n);
cudaError_t err = cudaGetLastError();
if (err != cudaSuccess) {
    fprintf(stderr, "Launch failed: %s\n", cudaGetErrorString(err));
    return 1;
}
```

**Diagnostic:**
```
file.cu:42:5: warning: CG001: kernel launch is not followed by cudaGetLastError, cudaPeekAtLastError, or cudaDeviceSynchronize
  hint: add cudaGetLastError() after the launch to catch asynchronous launch failures
```

**Implementation:** Matches `CUDAKernelCallExpr` inside `CompoundStmt`. Walks sibling statements after the kernel call looking for the error-checking functions.

**Limitations:**
- Only checks direct sibling statements (not nested in conditionals or loops).
- 5-statement lookahead window may miss error checks placed further away.
- Does not detect error checks through helper functions (e.g., `CHECK_CUDA()`).

---

## CG002: Suspicious Kernel Launch Configuration

**Severity:** Warning

**Description:** Detects kernel launches with obviously invalid literal dimensions:
- Zero grid dimension: `kernel<<<0, N>>>(...)`
- Zero block dimension: `kernel<<<N, 0>>>(...)`
- Block size exceeding 1024: `kernel<<<N, 2048>>>(...)`

**Rationale:** A zero dimension guarantees no threads execute. Block sizes above 1024 exceed the hardware maximum for all current CUDA devices.

**Bad Examples:**
```cuda
compute<<<4, 0>>>(data);         // Zero block size
compute<<<0, 256>>>(data);       // Zero grid size
compute<<<1, 2048>>>(data);      // Exceeds max block size
```

**Diagnostic:**
```
file.cu:18:5: warning: CG002: kernel launch uses a zero block dimension
  hint: block dimension should be positive; check thread-block calculation
```

**Implementation:** Matches `CUDAKernelCallExpr`, extracts config arguments via `getConfig()`, checks if arguments are `IntegerLiteral` nodes with invalid values.

**Limitations:**
- Only warns on literal integer values (not variables or expressions).
- Does not check `dim3` struct members.
- Does not verify grid size is reasonable for the problem size.

---

## CG003: Host/Device Qualifier Misuse

**Severity:** Error

**Description:** Detects when a `__device__` or `__global__` function directly calls a function that lacks `__device__` or `__host__ __device__` qualifiers.

**Rationale:** Functions called from device code must be compiled for the GPU. Calling a host-only function from device code is a compile error in nvcc and indicates a programmer mistake.

**Bad Example:**
```cuda
void hostHelper(int x) { printf("%d\n", x); }

__device__ int compute(int x) {
    hostHelper(x);  // Error: host-only function called from device code
    return x * 2;
}
```

**Good Example:**
```cuda
__host__ __device__ int sharedHelper(int x) { return x * 2; }

__device__ int compute(int x) {
    return sharedHelper(x);  // OK: __host__ __device__ qualified
}
```

**Diagnostic:**
```
file.cu:27:5: error: CG003: __device__ function 'compute' calls function 'hostHelper' that is not marked __device__ or __host__ __device__
  hint: add __device__ or __host__ __device__ qualifier to 'hostHelper'
```

**Implementation:** Matches `FunctionDecl` with `CUDADeviceAttr` or `CUDAGlobalAttr`, traverses descendant `CallExpr` nodes, resolves direct callee, checks for device attributes.

**Limitations:**
- Only checks direct callees (not calls through function pointers).
- Skips builtin functions and compiler intrinsics.
- Does not warn on dependent template calls (deferred to instantiation).
- May produce false positives for overload sets where a device version exists.

---

## CG004: Unsafe Dynamic Shared Memory Usage

**Severity:** Warning

**Description:** Detects kernel launches where the kernel uses `extern __shared__` memory but the launch does not provide a dynamic shared-memory size (third launch parameter).

**Rationale:** When a kernel declares `extern __shared__` memory, the size must be specified at launch time. Omitting it defaults to 0 bytes, causing out-of-bounds shared memory access.

**Bad Example:**
```cuda
__global__ void kernel() {
    extern __shared__ float buf[];
    buf[threadIdx.x] = 1.0f;
}

kernel<<<blocks, threads>>>();  // Missing shared memory size!
```

**Good Example:**
```cuda
kernel<<<blocks, threads, threads * sizeof(float)>>>();  // Correct
```

**Diagnostic:**
```
file.cu:33:5: warning: CG004: kernel 'kernel' uses extern __shared__ memory, but launch does not provide a dynamic shared-memory size
  hint: use the third kernel launch parameter to specify dynamic shared memory size
```

**Implementation:** Two-matcher approach:
1. Collect `__global__` functions containing `extern __shared__` `VarDecl` nodes.
2. Match `CUDAKernelCallExpr`, check if callee is in the collected set, verify config has >= 3 arguments with non-zero third argument.

**Limitations:**
- Only tracks kernels defined in the same translation unit.
- Cannot detect when shared memory size is passed as 0 through a variable.
- Does not analyze template kernel instantiations.

---

## CG005: cudaMemcpy Direction Mismatch (Heuristic)

**Severity:** Warning

**Description:** Detects `cudaMemcpy` calls where the direction argument may not match the known pointer categories (device vs. host).

**Rationale:** Passing the wrong direction to `cudaMemcpy` causes undefined behavior. Common mistake: swapping `cudaMemcpyHostToDevice` and `cudaMemcpyDeviceToHost`.

**Bad Example:**
```cuda
float* d_data;
cudaMalloc(&d_data, bytes);
float* h_data = (float*)malloc(bytes);

// Wrong! d_data is device memory, but direction says DeviceToHost (copies TO host)
cudaMemcpy(d_data, h_data, bytes, cudaMemcpyDeviceToHost);
```

**Diagnostic:**
```
file.cu:51:5: warning: CG005: cudaMemcpy direction may not match known pointer categories
  hint: destination 'd_data' was allocated with cudaMalloc (device), but copy direction is DeviceToHost
```

**Implementation:** Tracks variables passed to `cudaMalloc` as device pointers. When `cudaMemcpy` is called, checks if the direction is consistent with known pointer categories.

**Limitations:**
- Only tracks intra-procedural pointer provenance.
- Only detects pointers directly passed to `cudaMalloc` (not through wrappers).
- Does not track host pointers beyond `malloc`/`new` (stack arrays are ignored).
- Only warns on high-confidence mismatches to minimize false positives.
- Pointer aliasing and reassignment are not tracked.
