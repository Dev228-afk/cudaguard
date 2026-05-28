// bad_missing_error_check.cu — Demonstrates CG001: Missing CUDA error check.
// Run: ./build/cudaguard --file examples/demo/bad_missing_error_check.cu -- -x cuda --cuda-gpu-arch=sm_75
// Expected: CG001 warning on the kernel launch below.

#include <cuda_runtime.h>

__global__ void scale(float* data, float factor, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        data[i] *= factor;
    }
}

int main() {
    float* d_data;
    int n = 2048;
    cudaMalloc(&d_data, n * sizeof(float));

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    // BUG: No cudaGetLastError() or cudaDeviceSynchronize() after launch.
    // If the kernel fails (e.g. invalid config), the error is silently lost.
    scale<<<blocks, threads>>>(d_data, 2.0f, n);

    cudaFree(d_data);
    return 0;
}
