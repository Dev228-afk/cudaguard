// CG001: Kernel launch followed immediately by return.
#include <cuda_runtime.h>

__global__ void init(float* data) {
    data[threadIdx.x] = 0.0f;
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));
    init<<<1, 256>>>(d);
    return 0;  // immediate return, no error check
}
