#include "AsyncExecutor.hpp"

#include <mutex>
#include <condition_variable>

class TaskGroupV2{
private:

    AsyncExecutor& executor_;
    int pending_ = 0;
    std::mutex mtx_;
    std::condition_variable cv_;


public:

    TaskGroupV2(AsyncExecutor& executor) : executor_(executor) {}

    TaskGroupV2(const TaskGroupV2&) = delete;
    TaskGroupV2& operator=(const TaskGroupV2&) = delete;

    TaskGroupV2(TaskGroupV2&&) = delete;
    TaskGroupV2& operator=(TaskGroupV2&&) = delete;



    ~TaskGroupV2(){
        wait();
    }

    template<typename F, typename... Args>
    auto run(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto promise = std::make_shared<std::promise<ReturnType>>();

        std::future<ReturnType> result = promise->get_future();

        {
            std::lock_guard<std::mutex> lock(mtx_);
            pending_++;
        }

        auto boundTask = std::bind(
            std::forward<F>(f),
            std::forward<Args>(args)...
        );

        try {
            executor_.async(
                [this, promise, task = std::move(boundTask)]() mutable{
                    try {
                        if constexpr(std::is_void_v<ReturnType>){
                            task();
                            promise->set_value();
                        }
                        else {
                            promise->set_value(task());
                        }
                    }
                    catch (...){
                        promise->set_exception(std::current_exception());
                    }
                    markDone();
                }
            );
        }
        catch(...){
            markDone();
            throw;
        }

        return result;
    }

    void wait(){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{
            return pending_ == 0;
        });
    }

    void markDone(){
        bool notifyWait = false;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            pending_--;
            if (pending_ == 0)
                notifyWait = true;
            
        }

        if (notifyWait)
            cv_.notify_all();
    }

};