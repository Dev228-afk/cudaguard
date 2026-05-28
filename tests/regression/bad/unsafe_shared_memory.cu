#include <cuda_runtime.h>

__global__ void reductionKernel(const float* input, float* output, int n) {
    extern __shared__ float sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    sdata[tid] = (i < n) ? input[i] : 0.0f;
    __syncthreads();

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

int main() {
    float *d_input, *d_output;
    int n = 1024;
    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    cudaMalloc(&d_input, n * sizeof(float));
    cudaMalloc(&d_output, blocks * sizeof(float));

    // CG004: Missing third launch parameter for dynamic shared memory
    reductionKernel<<<blocks, threads>>>(d_input, d_output, n);
    cudaGetLastError();

    cudaFree(d_input);
    cudaFree(d_output);
    return 0;
}
