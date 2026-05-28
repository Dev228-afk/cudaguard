// CG001: Kernel launch followed only by cudaFree (no error check).
#include <cuda_runtime.h>

__global__ void compute(int* data) {
    data[threadIdx.x] = threadIdx.x;
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));

    compute<<<1, 256>>>(d);
    cudaFree(d);
    return 0;
}
