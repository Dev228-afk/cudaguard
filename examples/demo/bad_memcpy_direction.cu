// bad_memcpy_direction.cu — Demonstrates CG005: cudaMemcpy direction mismatch.
// Run: ./build/cudaguard --file examples/demo/bad_memcpy_direction.cu -- -x cuda --cuda-gpu-arch=sm_75
// Expected: CG005 warning on the cudaMemcpy call below.

#include <cuda_runtime.h>
#include <cstdlib>

int main() {
    const int n = 1024;
    size_t bytes = n * sizeof(float);

    float* h_data = (float*)malloc(bytes);
    float* d_data;
    cudaMalloc(&d_data, bytes);

    // BUG: d_data is device memory (allocated with cudaMalloc), but
    // cudaMemcpyDeviceToHost means "copy FROM device TO host".
    // Since d_data is the destination, this should be cudaMemcpyHostToDevice.
    cudaMemcpy(d_data, h_data, bytes, cudaMemcpyDeviceToHost);

    free(h_data);
    cudaFree(d_data);
    return 0;
}
