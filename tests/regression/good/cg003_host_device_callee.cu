// Good: __device__ function calling __host__ __device__ function — valid.
#include <cuda_runtime.h>

__host__ __device__ int shared_math(int x) {
    return x * x + x;
}

__device__ int device_work(int x) {
    return shared_math(x) - 1;
}

__global__ void kernel(int* out) {
    out[threadIdx.x] = device_work(threadIdx.x);
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
