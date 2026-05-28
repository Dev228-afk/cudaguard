// CG005: Source is device pointer but direction says HostToDevice.
#include <cuda_runtime.h>
#include <cstdlib>

int main() {
    float* h_buf = (float*)malloc(512 * sizeof(float));
    float* d_buf;
    cudaMalloc(&d_buf, 512 * sizeof(float));

    // BUG: d_buf is source, direction is HostToDevice.
    // HostToDevice means src=host, dst=device. But here src=device.
    cudaMemcpy(h_buf, d_buf, 512 * sizeof(float), cudaMemcpyHostToDevice);

    free(h_buf);
    cudaFree(d_buf);
    return 0;
}
