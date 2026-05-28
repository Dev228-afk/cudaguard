#include <cuda_runtime.h>
#include <cstdio>

// Host-only function (no __device__ qualifier)
void hostOnlyLogger(int x) {
    printf("value=%d\n", x);
}

// Host-only helper
int hostCompute(int a, int b) {
    return a + b;
}

// CG003: __device__ function calling host-only functions
__device__ int deviceCompute(int x) {
    hostOnlyLogger(x);
    int result = hostCompute(x, x);
    return result * 2;
}

// Correct: __host__ __device__ function
__host__ __device__ int sharedHelper(int x) {
    return x * 3;
}

__device__ int correctDeviceFunc(int x) {
    return sharedHelper(x);
}

__global__ void kernel(int* data) {
    int tid = threadIdx.x;
    data[tid] = deviceCompute(tid);
}

int main() {
    int* d_data;
    cudaMalloc(&d_data, 256 * sizeof(int));
    kernel<<<1, 256>>>(d_data);
    cudaGetLastError();
    cudaFree(d_data);
    return 0;
}
