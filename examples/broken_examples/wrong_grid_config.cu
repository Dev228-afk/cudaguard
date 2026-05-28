#include <cuda_runtime.h>

__global__ void compute(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        data[i] = data[i] * 2.0f;
    }
}

int main() {
    float* d_data;
    int n = 1024;
    cudaMalloc(&d_data, n * sizeof(float));

    // CG002: Zero block dimension
    compute<<<4, 0>>>(d_data, n);
    cudaGetLastError();

    // CG002: Zero grid dimension
    compute<<<0, 256>>>(d_data, n);
    cudaGetLastError();

    // CG002: Suspicious block size above 1024
    compute<<<1, 2048>>>(d_data, n);
    cudaGetLastError();

    cudaFree(d_data);
    return 0;
}
