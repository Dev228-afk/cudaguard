// CG003: Nested __device__ function calling host-only helper.
#include <cuda_runtime.h>

void hostFormat(int x) {
    (void)x;
}

__device__ int innerDevice(int x) {
    hostFormat(x);
    return x + 1;
}

__device__ int outerDevice(int x) {
    return innerDevice(x) * 2;
}

__global__ void kernel(int* out) {
    out[threadIdx.x] = outerDevice(threadIdx.x);
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
