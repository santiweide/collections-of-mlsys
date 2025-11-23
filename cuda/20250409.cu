
/// MxK KxN -> MxN
__global__ void gemm(float* a, float* b, float* c, int M, int N, int K) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    int idy = threadIdx.y + blockIdx.y * blockDim.y;
    
    float val = 0.0f;
    for (int k = 0; k < K; ++k) {
        val += a[idx * N + k] * b[k * K + j];
    }

    c[idx][idy] = val;

    return;
}

const int BLOCK_SIZE = 64;
dim3 grid((N + BLOCK_SIZE - 1)/BLOCK_SIZE);
dim3 block(BLOCK_SIZE);

// warp level reduce sum
__global__ void reduce_sum(float* a, float* ans, int N) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    float val = (idx < N) ? a[idx] : 0.0f;
    for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
        val += __shfl_down_sync(0xFFFFFFFF, val, offset);
    } 
    // 因为warp内Thread执行是sync的
    // 我们把warp level reduce的结果先保存到shared_mem中
    int warpNum = (blockDim.x + warpSize - 1)/ warpSize;
    int warpId = threadIdx.x / warpSize;
    int laneId = threadIdx.x % warpSize;
    __shared__ float smem[warpNum];
    if (laneId == 0) { 
        smem[warpId] = val;
    }
    // 一个block可能有多个warp，所以要在warp之间的计算做sync
    __syncthread();

    // block level reduce
    if (warpId == 0) {
        val = (laneId < warpNum) ? smem[laneId] : 0.0;
        // 一个block只有一个warpId==0，所以不用sync thread
        // 这里从warpSize开始reduce，
        // 是考虑block max thread num < 1024, 所以warpNum <= warpSize 
        // 并且warpNum不一定是2的幂次，所以用warpSize比较好
        for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
            val += __shfl_down_sync(0xFFFFFFFF, val, offset);
        } 
        if (laneId == 0) {
            atomicAdd(ans, val); // 也可以换成grid level的reduce
        }
    }
}

#include <cfloat.h>
__global__ void reduce_max(float* a, float* max_val, int N) {
    // step1 reduce within each warp, store in the shared memory
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    float val = (idx < N) ? a[idx] : FLT_MIN;
    for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
        val = fmaxf(val, __shfl_down_sync(0xFFFFFFFF, val, offset)); // down means plus
    }
    int warpNum = (warpSize - 1 + blockDim.x) / warpSize;
    int warpId = threadIdx.x / warpSize;
    int laneId = threadIdx.x % warpSize;
    __shared__ float smem[warpNum];
    if (laneId == 0) { // fir
        smem[warpId] = val;
    }
    __syncthread();

    // step2 reduce within each block, do atomic comparison with max_val
    if (warpId == 0) {
        val = (laneId < warpNum) ? smem[laneId] : 0.0;
        for (int offset = warpSize >> 1; offset > 0; offset >>= 1) {
            val = fmaxf(val, __shfl_down_sync(0xFFFFFFFF, val, offset));
        }
        if (laneId == 0) {
            atomicMax(max_val, val);
        }
    }
}

// grid level reduction
// needs 2 kernel