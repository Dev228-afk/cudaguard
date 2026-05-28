// Good: Launch config uses variables (not literals) — no CG002.
// CG002 only checks literal values; variable expressions are skipped.
#include <cuda_runtime.h>

__global__ void kernel(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] = 0.0f;
}

int main() {
    float* d;
    int n = 4096;
    cudaMalloc(&d, n * sizeof(float));

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    kernel<<<blocks, threads>>>(d, n);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
