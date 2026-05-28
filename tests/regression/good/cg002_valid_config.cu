// Good: Valid launch config (256 threads, non-zero grid) — no CG002 warning.
#include <cuda_runtime.h>

__global__ void kernel(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] = 0.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 2048 * sizeof(float));
    kernel<<<8, 256>>>(d, 2048);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
