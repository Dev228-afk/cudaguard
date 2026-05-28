#include <cuda_runtime.h>
#include <cstdio>

__global__ void reduce(const float* input, float* output, int n) {
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
    const int n = 1024;
    size_t inputBytes = n * sizeof(float);
    int threads = 256;
    int blocks = (n + threads - 1) / threads;
    size_t outputBytes = blocks * sizeof(float);
    size_t sharedBytes = threads * sizeof(float);

    float *d_input, *d_output;
    cudaMalloc(&d_input, inputBytes);
    cudaMalloc(&d_output, outputBytes);

    reduce<<<blocks, threads, sharedBytes>>>(d_input, d_output, n);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr, "Kernel error: %s\n", cudaGetErrorString(err));
        return 1;
    }
    cudaDeviceSynchronize();

    cudaFree(d_input);
    cudaFree(d_output);
    return 0;
}
