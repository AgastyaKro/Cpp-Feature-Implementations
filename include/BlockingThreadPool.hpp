#pragma once
#include "BlockingCountingMutexSemaphore.hpp"
#include "BlockingSemaphoreQueue.hpp"

#include <functional>
#include <vector>
#include <thread>
#include <exception>
#include <future>
#include <optional>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <memory>

class BlockingThreadPool {
private:
    using Task = std::function<void()>;
    BlockingSemaphoreQueue<std::optional<Task>> tasks_; 
    std::vector<std::thread> workers_;

    std::mutex submitMtx_;
    bool stopping_ = false;

public:

    explicit BlockingThreadPool(std::size_t amountOfWorkers, std::size_t queueCapacity) : tasks_(queueCapacity){
        if (amountOfWorkers == 0)
            throw std::invalid_argument("Amount of workers can't be 0");
        
        for (std::size_t i = 0; i < amountOfWorkers; i++){
            workers_.emplace_back([this](){
                workersLoop();
            });
        }
    }

    BlockingThreadPool(const BlockingThreadPool&) = delete;
    BlockingThreadPool& operator=(const BlockingThreadPool&) = delete;

    BlockingThreadPool(BlockingThreadPool&&) = delete;
    BlockingThreadPool& operator=( BlockingThreadPool&&) = delete;


    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto boundTask = std::bind(
            std::forward<F>(f),
            std::forward<Args>(args)...
        );

        auto packaged = std::make_shared<std::packaged_task<ReturnType()>>(
            std::move(boundTask)
        );

        std::future<ReturnType> result = packaged->get_future();

        Task wrapper = [packaged](){
            (*packaged)();
        };

        {
            std::lock_guard<std::mutex> lock(submitMtx_);

            if (stopping_){
                throw std::runtime_error("Can't submit task when threadpool is stopping");
            }
            tasks_.push(std::move(wrapper));
        }

        return result;

    }


    ~BlockingThreadPool(){
        shutdown();
    }

    void shutdown(){
        {
            std::lock_guard<std::mutex> lock(submitMtx_);

            if (stopping_)
                return;

            stopping_ = true;

            for (std::size_t i = 0; i < workers_.size(); i++){
                tasks_.push(std::nullopt);
            }
        }

        for (std::thread& worker : workers_){
            if (worker.joinable()){
                worker.join();
            }
        }
    }

    void workersLoop(){
        while (true){
            std::optional<Task> maybeTask = tasks_.pop();
            if (!maybeTask.has_value()){
                break;
            }
            Task task = std::move(*maybeTask);
            task();
        }
    }

};