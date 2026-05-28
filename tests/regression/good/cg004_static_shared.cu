// Good: Static shared memory (not extern) — no CG004.
#include <cuda_runtime.h>

__global__ void kernel(float* data) {
    __shared__ float buf[256];  // static — size known at compile time
    buf[threadIdx.x] = data[threadIdx.x];
    __syncthreads();
    data[threadIdx.x] = buf[threadIdx.x] * 2.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));
    kernel<<<1, 256>>>(d);  // No third param needed for static shared
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
