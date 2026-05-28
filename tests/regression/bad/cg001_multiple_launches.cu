// CG001: Multiple kernel launches, all missing error checks.
#include <cuda_runtime.h>

__global__ void kernelA(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] += 1.0f;
}

__global__ void kernelB(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] *= 2.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 1024 * sizeof(float));

    kernelA<<<4, 256>>>(d, 1024);
    // no error check

    kernelB<<<4, 256>>>(d, 1024);
    // no error check

    cudaFree(d);
    return 0;
}
