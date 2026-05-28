// CG003: __device__ function calling multiple host-only functions.
#include <cuda_runtime.h>
#include <cstdio>

void hostPrint(int x) { printf("%d\n", x); }
int hostSquare(int x) { return x * x; }
float hostSqrt(float x) { return x * 0.5f; }

__device__ int broken(int x) {
    hostPrint(x);
    int sq = hostSquare(x);
    float r = hostSqrt((float)sq);
    return (int)r;
}

__global__ void kernel(int* out) {
    out[threadIdx.x] = broken(threadIdx.x);
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d);
    cudaGetLastError();
    cudaFree(d);
    return 0;
}
