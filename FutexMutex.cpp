#include <atomic>
#include <sys/syscall.h>
#include <linux/futex.h>




class FutexMutex {
public:
    FutexMutex() : state_(0) {}




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