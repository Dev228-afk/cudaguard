// Good: Block size exactly 1024 — valid (max but not over).
#include <cuda_runtime.h>

__global__ void kernel(float* data) {
    data[threadIdx.x] = 1.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 1024 * sizeof(float));
    kernel<<<1, 1024>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
