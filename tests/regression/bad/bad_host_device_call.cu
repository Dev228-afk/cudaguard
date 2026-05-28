#include <cuda_runtime.h>
#include <cstdio>

void hostLogger(const char* msg) {
    printf("%s\n", msg);
}

int hostMath(int a, int b) {
    return a * b + a;
}

__device__ int brokenDeviceFunc(int x) {
    hostLogger("computing");
    return hostMath(x, x + 1);
}

__global__ void kernel(int* out) {
    out[threadIdx.x] = brokenDeviceFunc(threadIdx.x);
}

int main() {
    int* d_out;
    cudaMalloc(&d_out, 256 * sizeof(int));
    kernel<<<1, 256>>>(d_out);
    cudaGetLastError();
    cudaFree(d_out);
    return 0;
}
