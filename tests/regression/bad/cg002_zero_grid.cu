// CG002: Zero grid dimension — guaranteed no threads execute.
#include <cuda_runtime.h>

__global__ void noop(float* data) {
    data[threadIdx.x] = 0.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));

    noop<<<0, 256>>>(d);
    cudaGetLastError();

    cudaFree(d);
    return 0;
}
