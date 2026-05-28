// Good: Launch provides dynamic shared memory size — no CG004.
#include <cuda_runtime.h>

__global__ void reduce(const float* input, float* output, int n) {
    extern __shared__ float sdata[];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    sdata[tid] = (i < n) ? input[i] : 0.0f;
    __syncthreads();
    if (tid == 0) output[blockIdx.x] = sdata[0];
}

int main() {
    float *d_in, *d_out;
    cudaMalloc(&d_in, 1024 * sizeof(float));
    cudaMalloc(&d_out, 4 * sizeof(float));

    // Correct: third parameter provides shared mem size
    reduce<<<4, 256, 256 * sizeof(float)>>>(d_in, d_out, 1024);
    cudaGetLastError();

    cudaFree(d_in);
    cudaFree(d_out);
    return 0;
}
