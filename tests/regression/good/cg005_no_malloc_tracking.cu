// Good: Pointer source unknown (not tracked) — no CG005 warning.
// CG005 only warns on high-confidence cases where provenance is tracked.
#include <cuda_runtime.h>

void doTransfer(float* dst, float* src, size_t bytes) {
    // Neither pointer tracked via cudaMalloc in this scope
    cudaMemcpy(dst, src, bytes, cudaMemcpyDeviceToHost);
}

int main() {
    float* a;
    float* b;
    cudaMalloc(&a, 256 * sizeof(float));
    cudaMalloc(&b, 256 * sizeof(float));
    // No mismatched direction for tracked pointers
    cudaMemcpy(b, a, 256 * sizeof(float), cudaMemcpyDeviceToDevice);
    cudaFree(a);
    cudaFree(b);
    return 0;
}
