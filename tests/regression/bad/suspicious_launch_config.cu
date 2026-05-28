#include <cuda_runtime.h>

__global__ void processData(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        data[i] = data[i] + 1.0f;
    }
}

int main() {
    float* d_data;
    int n = 1024;
    cudaMalloc(&d_data, n * sizeof(float));

    // Bad: zero block size
    processData<<<4, 0>>>(d_data, n);
    cudaGetLastError();

    // Bad: block size > 1024
    processData<<<1, 2048>>>(d_data, n);
    cudaGetLastError();

    cudaFree(d_data);
    return 0;
}
