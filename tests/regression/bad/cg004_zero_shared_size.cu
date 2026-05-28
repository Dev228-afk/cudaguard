// CG004: Third parameter is explicit zero — same as omitting it.
#include <cuda_runtime.h>

__global__ void prefixSum(float* data, int n) {
    extern __shared__ float shared[];
    int tid = threadIdx.x;
    shared[tid] = data[tid];
    __syncthreads();
    data[tid] = shared[tid];
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));

    // Third param is 0 — effectively no shared memory allocated
    prefixSum<<<1, 256, 0>>>(d, 256);
    cudaGetLastError();

    cudaFree(d);
    return 0;
}
