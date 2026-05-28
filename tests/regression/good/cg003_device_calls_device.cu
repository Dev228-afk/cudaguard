// Good: __device__ function calling another __device__ function — no CG003.
#include <cuda_runtime.h>

__device__ int helper(int x) {
    return x * 2;
}

__device__ int compute(int x) {
    return helper(x) + 1;
}

__global__ void kernel(int* out) {
    out[threadIdx.x] = compute(threadIdx.x);
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
