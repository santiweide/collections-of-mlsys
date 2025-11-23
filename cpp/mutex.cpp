#include <atomic>
#include <mutex>
#include <condition_variable>

// 在没有竞争（Contention）时，完全在用户态（User Space）操作，速度极快；
// 只有在发生竞争时，才陷入内核态（Kernel Space）挂起线程。

// State:
// 0 (Unlocked): 锁是自由的，没有人持有。
// 1 (Locked, No Waiters): 锁被持有，但是没有其他线程在等待（或者持有者认为没有）。这是“快路径”状态。
// 2 (Locked, Contention): 锁被持有，且有至少一个线程在等待（或者曾经有线程试图获取锁失败了）。这是“慢路径”状态。
class MiniMutex {
public:
    MiniMutex() : state(0) {}

    void lock() {
        int expected = 0;
        if (state.compare_exchange_strong(expected, 1, std::memory_order_acquire))
            return;

        for (int i = 0; i < 100; ++i) {
            expected = 0;
            if (state.compare_exchange_strong(expected, 1, std::memory_order_acquire))
                return;
            __builtin_ia32_pause();
        }

        std::unique_lock<std::mutex> lk(wait_mtx);

        while (true) {
            expected = 0;
            if (state.compare_exchange_strong(expected, 1, std::memory_order_acquire))
                return;

            cv.wait(lk);
        }
    }

    void unlock() {
        state.store(0, std::memory_order_release);

        std::lock_guard<std::mutex> lk(wait_mtx);
        cv.notify_one();
    }

private:
    std::atomic<int> state;
    std::mutex wait_mtx;
    std::condition_variable cv;
};



lock():
    # 1. 快路径 (Fast Path)
    # 尝试原子地将 state 从 0 变为 1。
    # 如果成功，说明锁之前是空的，现在我拿到了，直接返回。
    # 这一步完全在用户态，非常快（几纳秒）。
    if CAS(state, 0, 1) success:
        return

    # 2. 慢路径 (Slow Path)
    # 如果 CAS 失败，说明锁被别人占用了，进入循环重试。
    while CAS(state, 0, 1) failed:
        
        # 3. 标记竞争 (Mark Contention)
        # 如果当前 state 是 1，说明持有者不知道有人在等。
        # 我把它改成 2，意思是："大哥，不仅你拿着锁，门口还排队呢！释放时记得叫醒我。"
        if state == 1:
            state = 2        

        # 4. 陷入睡眠 (Wait)
        # futex_wait(addr, val) 的含义是：
        # "如果 addr 指向的值仍然是 val，就把当前线程挂起（进入内核睡眠状态）"。
        # 如果 state 不等于 2 了（比如刚才锁被释放了），这个调用会立即返回，不睡眠，继续循环尝试 CAS。
        futex_wait(state, 2)

unlock():
    # 1. 快路径 (Fast Path)
    # 如果 state 还是 1，说明虽然我拿着锁，但并没有人把状态改成 2。
    # 也就是说，没有人在排队（或者由于竞争，没人成功标记为 2）。
    # 直接把锁置为 0。不需要进内核，不需要系统调用。快！
    if state == 1:
        state = 0
    
    # 2. 慢路径 (Slow Path)
    else:
        # state 不为 1 (通常就是 2)，说明有人在等锁（或者曾经标记过）。
        # 首先释放锁。
        state = 0
        
        # 然后调用内核，唤醒一个在 futex_wait 上睡眠的线程。
        # 这是一个系统调用。
        futex_wake_one()