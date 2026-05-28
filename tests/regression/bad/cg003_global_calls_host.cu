// CG003: __global__ kernel directly calling a host-only function.
#include <cuda_runtime.h>

int hostOnlyMath(int a, int b) {
    return a * b + a - b;
}

__global__ void kernel(int* data, int n) {
    int i = threadIdx.x;
    if (i < n) {
        data[i] = hostOnlyMath(i, n);
    }
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d, 256);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
