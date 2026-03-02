#include <mutex>
#include <condition_variable>
#include <chrono>

class CountingSemaphore{

private:
    int count;
    std::condition_variable cv;
    std::mutex mtx;


public:
    CountingSemaphore(int count_) : count(count_){}

    void acquire(){
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{
            return count > 0;
        });
        count--;

    };

    bool try_acquire(){
        std::unique_lock<std::mutex> lock(mtx);

        if (count > 0){
            count--;
            return true;
        }
        return false;
    }

    bool acquire_for(std::chrono::milliseconds timeout){
        std::unique_lock<std::mutex> lock(mtx);
        if (!cv.wait_for(lock, timeout, [this]{
            count > 0;
        })){
            return false;
        }
        count--;
        return true;
    }

    void release(){
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        lock.unlock();
        cv.notify_one();
    }

};
