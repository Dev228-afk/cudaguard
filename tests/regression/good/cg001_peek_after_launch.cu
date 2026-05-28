// Good: cudaPeekAtLastError after launch satisfies CG001.
#include <cuda_runtime.h>

__global__ void kernel(int* data) {
    data[threadIdx.x] = threadIdx.x;
}

int main() {
    int* d;
    cudaMalloc(&d, 256 * sizeof(int));
    kernel<<<1, 256>>>(d);
    cudaPeekAtLastError();
    cudaFree(d);
    return 0;
}
