// CG004: Two kernels with extern __shared__, both launched without shared mem size.
#include <cuda_runtime.h>

__global__ void reduce(const float* input, float* output, int n) {
    extern __shared__ float sdata[];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    sdata[tid] = (i < n) ? input[i] : 0.0f;
    __syncthreads();
    if (tid == 0) output[blockIdx.x] = sdata[0];
}

__global__ void scan(int* data, int n) {
    extern __shared__ int temp[];
    int tid = threadIdx.x;
    temp[tid] = data[tid];
    __syncthreads();
    if (tid > 0) data[tid] = temp[tid] + temp[tid - 1];
}

int main() {
    float *d_in, *d_out;
    int* d_data;
    cudaMalloc(&d_in, 1024 * sizeof(float));
    cudaMalloc(&d_out, 4 * sizeof(float));
    cudaMalloc(&d_data, 256 * sizeof(int));

    // Both missing third launch parameter
    reduce<<<4, 256>>>(d_in, d_out, 1024);
    cudaGetLastError();

    scan<<<1, 256>>>(d_data, 256);
    cudaGetLastError();

    cudaFree(d_in);
    cudaFree(d_out);
    cudaFree(d_data);
    return 0;
}
