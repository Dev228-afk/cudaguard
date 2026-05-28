// CG005: Direction says DeviceToDevice but destination is host.
#include <cuda_runtime.h>
#include <cstdlib>

int main() {
    float* h_dst = (float*)malloc(256 * sizeof(float));
    float* d_src;
    cudaMalloc(&d_src, 256 * sizeof(float));

    // BUG: h_dst is host memory, but direction says DeviceToDevice.
    cudaMemcpy(h_dst, d_src, 256 * sizeof(float), cudaMemcpyDeviceToDevice);

    free(h_dst);
    cudaFree(d_src);
    return 0;
}
