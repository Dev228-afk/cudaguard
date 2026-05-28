// CG002: Block size 2048 exceeds hardware maximum of 1024.
#include <cuda_runtime.h>

__global__ void process(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] = data[i] * 2.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 4096 * sizeof(float));

    process<<<2, 2048>>>(d, 4096);
    cudaGetLastError();

    cudaFree(d);
    return 0;
}
