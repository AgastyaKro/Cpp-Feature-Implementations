#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

class ThreadPool{
    private:
        std::vector<std::thread> workers;
        std::vector<std::function<void()>> tasks;

        std::mutex mtx;
        std::condition_variable cv;
        bool stop = false;


    public:


};