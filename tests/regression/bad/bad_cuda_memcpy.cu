#include <cuda_runtime.h>
#include <cstdlib>

int main() {
    float* h_data = (float*)malloc(1024 * sizeof(float));
    float* d_data;
    cudaMalloc(&d_data, 1024 * sizeof(float));

    // CG005: Direction mismatch - d_data is device memory but direction says DeviceToHost
    // (destination is device, should be HostToDevice)
    cudaMemcpy(d_data, h_data, 1024 * sizeof(float), cudaMemcpyDeviceToHost);

    free(h_data);
    cudaFree(d_data);
    return 0;
}
