// bad_shared_memory.cu — Demonstrates CG004: extern __shared__ without launch-time size.
// Run: ./build/cudaguard --file examples/demo/bad_shared_memory.cu -- -x cuda --cuda-gpu-arch=sm_75
// Expected: CG004 warning on the kernel launch below.

#include <cuda_runtime.h>

__global__ void histogram(const int* input, int* bins, int n) {
    // Dynamic shared memory — size must be specified at launch time.
    extern __shared__ int localBins[];

    int tid = threadIdx.x;
    localBins[tid] = 0;
    __syncthreads();

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        atomicAdd(&localBins[input[i]], 1);
    }
    __syncthreads();

    atomicAdd(&bins[tid], localBins[tid]);
}

int main() {
    int *d_input, *d_bins;
    int n = 1024;
    cudaMalloc(&d_input, n * sizeof(int));
    cudaMalloc(&d_bins, 256 * sizeof(int));

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    // BUG: Missing third launch parameter for dynamic shared memory size.
    // Should be: histogram<<<blocks, threads, 256 * sizeof(int)>>>(...)
    histogram<<<blocks, threads>>>(d_input, d_bins, n);
    cudaGetLastError();

    cudaFree(d_input);
    cudaFree(d_bins);
    return 0;
}
