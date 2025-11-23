
__global__ void count_equal_kernel(const int* input, int* output, int N, int K) {
    __shared__ int warp_sums[32];

    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int warpId = threadIdx.x / warpSize;
    int laneId = threadIdx.x % warpSize;

    int warp_sum = 0; // block wise
    if (tid < N) {
        if (input[tid] == K) {
            warp_sum = 1;
        }
    }

    for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
        warp_sum += __shfl_down_sync(0xFFFFFFFF, warp_sum, offset);
    }
    if (laneId == 0) {
        warp_sums[warpId] = warp_sum;
    }
    __syncthreads();

    if (warpId == 0) {
        int block_sum = 0;
        int warpNum = blockDim.x / warpSize;
        if (laneId < warpNum) {
            block_sum = warp_sums[laneId];
        }
        for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
            block_sum += __shfl_down_sync(0xFFFFFFFF, block_sum, offset);
        }

        if (laneId == 0) atomicAdd(output, block_sum); 

    }


}
