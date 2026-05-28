// Good: cudaDeviceSynchronize after launch satisfies CG001.
#include <cuda_runtime.h>

__global__ void kernel(float* data, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] += 1.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 1024 * sizeof(float));
    kernel<<<4, 256>>>(d, 1024);
    cudaDeviceSynchronize();
    cudaFree(d);
    return 0;
}
