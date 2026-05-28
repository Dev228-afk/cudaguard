// CG003: __device__ function calling a plain function that uses stdio.
#include <cuda_runtime.h>

void printResult(float x) {
    (void)x;  // would use printf
}

__device__ float deviceCalc(float x) {
    float result = x * x;
    printResult(result);
    return result;
}

__global__ void kernel(float* out) {
    out[threadIdx.x] = deviceCalc((float)threadIdx.x);
}

int main() {
    float* d;
    cudaMalloc(&d, 256 * sizeof(float));
    kernel<<<1, 256>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
