// good_vector_add.cu — Clean CUDA code that passes all CudaGuard checks.
// Run: ./build/cudaguard --file examples/demo/good_vector_add.cu -- -x cuda --cuda-gpu-arch=sm_75
// Expected: 0 errors, 0 warnings

#include <cuda_runtime.h>

__global__ void vectorAdd(const float* a, const float* b, float* c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}

int main() {
    const int n = 1024;
    size_t bytes = n * sizeof(float);

    float *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);

    float* h_a = (float*)malloc(bytes);
    float* h_b = (float*)malloc(bytes);

    cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice);

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    vectorAdd<<<blocks, threads>>>(d_a, d_b, d_c, n);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        return 1;
    }
    cudaDeviceSynchronize();

    float* h_c = (float*)malloc(bytes);
    cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost);

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    free(h_a);
    free(h_b);
    free(h_c);
    return 0;
}
