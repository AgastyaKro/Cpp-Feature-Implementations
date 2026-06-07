#include <atomic>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <cerrno>





class FutexMutex {
public:
    FutexMutex() : state_(0) {}

    void lock(){
        int expected = 0;
        // fast path
        if (state_.compare_exchange_strong(expected, 1, std::memory_order_acquire, std::memory_order_relaxed)){
            return;
        }
        // slow path
        for (;;){
            if (state_.exchange(2, std::memory_order_acquire) == 0){
                return;
            }
            futex_wait(&state_, 2);
        }
    }

    void unlock(){
        if (state.exchange(0, std::memory_order_release) == 2){
            futex_wake(&state_, 1);
        }
    }


private:
    std::atomic<int> state_;

    static int futex_wait(std::atomic<int*> addr, int expected){
        int res = syscall(
            SYS_futex,
            reinterpret_cast<int*>(addr),
            FUTEX_WAIT_PRIVATE,
            expected,
            nullptr,
            nullptr,
            0
        );

        return res;
    }

    static void futex_wake(std::atomic<int*> addr, int count){
        return syscall(
            SYS_futex,
            reinterpret_cast<int*>(addr),
            FUTEX_WAKE_PRIVATE,
            count,
            nullptr,
            nullptr,
            0
        )
    }





};