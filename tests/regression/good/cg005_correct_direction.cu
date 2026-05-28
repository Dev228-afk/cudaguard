// Good: cudaMemcpy with correct direction — no CG005.
#include <cuda_runtime.h>
#include <cstdlib>

int main() {
    float* h_data = (float*)malloc(1024 * sizeof(float));
    float* d_data;
    cudaMalloc(&d_data, 1024 * sizeof(float));

    // Correct: host -> device
    cudaMemcpy(d_data, h_data, 1024 * sizeof(float), cudaMemcpyHostToDevice);

    // Correct: device -> host
    cudaMemcpy(h_data, d_data, 1024 * sizeof(float), cudaMemcpyDeviceToHost);

    free(h_data);
    cudaFree(d_data);
    return 0;
}
