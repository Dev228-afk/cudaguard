#include <cuda_runtime.h>

__global__ void addKernel(float* c, const float* a, const float* b, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}

int main() {
    float *d_a, *d_b, *d_c;
    int n = 2048;
    size_t bytes = n * sizeof(float);

    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    addKernel<<<blocks, threads>>>(d_c, d_a, d_b, n);
    // Missing: cudaGetLastError() or similar

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    return 0;
}
