// bad_host_device_call.cu — Demonstrates CG003: __device__ function calls host-only function.
// Run: ./build/cudaguard --file examples/demo/bad_host_device_call.cu -- -x cuda --cuda-gpu-arch=sm_75
// Expected: CG003 error on the call to 'logValue' inside 'deviceCompute'.

#include <cuda_runtime.h>
#include <cstdio>

// This function has no __device__ qualifier — it only runs on host.
void logValue(int x) {
    printf("value = %d\n", x);
}

// BUG: __device__ function calls a host-only function.
// This would fail during nvcc compilation with a cross-execution-space call error.
__device__ int deviceCompute(int x) {
    logValue(x);       // ERROR: host-only callee
    return x * x;
}

__global__ void kernel(int* output, int n) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i < n) {
        output[i] = deviceCompute(i);
    }
}

int main() {
    int* d_out;
    cudaMalloc(&d_out, 256 * sizeof(int));
    kernel<<<1, 256>>>(d_out, 256);
    cudaGetLastError();
    cudaFree(d_out);
    return 0;
}
