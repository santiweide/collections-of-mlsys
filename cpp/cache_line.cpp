#include <iostream>
#include <vector>
#include <chrono>

// 定义一个大矩阵 10000 x 10000
// int 是 4 字节，总大小约 400MB，远超 L3 Cache，确保必须读内存
const int ROWS = 10000;
const int COLS = 10000;

int main() {
    // 使用 vector 在堆上分配，防止爆栈
    std::vector<std::vector<int>> matrix(ROWS, std::vector<int>(COLS, 1));

    long long sum = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            sum += matrix[i][j];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "行遍历 (Cache Friendly): " << diff.count() << " s" << std::endl;

    // 重置 sum
    sum = 0;

    // -------------------------------------------------
    // 测试 2: 缓存不友好 (Column-Major) - 跳跃访问
    // 内存访问模式: [0][0], [1][0], [2][0] ... 每次跳跃 10000 * 4 字节
    // 每次访问几乎都会触发 Cache Miss
    // -------------------------------------------------
    start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < COLS; ++j) {       // 注意这里外层是 j (列)
        for (int i = 0; i < ROWS; ++i) {   // 内层是 i (行)
            sum += matrix[i][j];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "列遍历 (Cache Unfriendly): " << diff.count() << " s" << std::endl;

    return 0;
}