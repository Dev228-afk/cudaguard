// CG002: Block size 4096 — extremely oversized.
#include <cuda_runtime.h>

__global__ void fill(int* data) {
    data[threadIdx.x] = 1;
}

int main() {
    int* d;
    cudaMalloc(&d, 4096 * sizeof(int));

    fill<<<1, 4096>>>(d);
    cudaGetLastError();

    cudaFree(d);
    return 0;
}
