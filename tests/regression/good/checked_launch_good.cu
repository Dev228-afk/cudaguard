#include <cuda_runtime.h>

__global__ void scale(float* data, float factor, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        data[i] *= factor;
    }
}

__host__ __device__ int helper(int x) {
    return x + 1;
}

__device__ int deviceHelper(int x) {
    return helper(x) * 2;
}

int main() {
    float* d_data;
    int n = 512;
    cudaMalloc(&d_data, n * sizeof(float));

    int threads = 128;
    int blocks = (n + threads - 1) / threads;

    scale<<<blocks, threads>>>(d_data, 2.0f, n);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        cudaFree(d_data);
        return 1;
    }
    cudaDeviceSynchronize();

    cudaFree(d_data);
    return 0;
}
