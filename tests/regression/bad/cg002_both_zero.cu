// CG002: Both grid and block are zero — doubly invalid.
#include <cuda_runtime.h>

__global__ void empty(float* data) {
    data[threadIdx.x] = 0.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));

    empty<<<0, 0>>>(d);
    cudaGetLastError();

    cudaFree(d);
    return 0;
}
