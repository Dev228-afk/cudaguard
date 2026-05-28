// CG001: Kernel launch inside a loop body without error check after.
#include <cuda_runtime.h>

__global__ void iterate(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] += 1.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 1024 * sizeof(float));

    for (int step = 0; step < 10; step++) {
        iterate<<<4, 256>>>(d, 1024);
        // no error check inside loop
    }

    cudaFree(d);
    return 0;
}
