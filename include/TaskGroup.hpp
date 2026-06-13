#pragma once

#include "AsyncExecutor.hpp"
#include <vector>
#include <future>
#include <utility>


class TaskGroup{
private:
    AsyncExecutor& executor_;
    std::vector<std::future<void>> futures_;


public:

    TaskGroup(AsyncExecutor& exec) : executor_(exec) {}

    TaskGroup(const TaskGroup&) = delete;
    TaskGroup(TaskGroup&&) = delete;
    TaskGroup& operator=(const TaskGroup&) = delete;
    TaskGroup& operator=(TaskGroup&&) = delete;

    template <typename F, typename... Args>
    void run(F&& f, Args&&... args){
        futures_.push_back(executor_.async(
                        std::forward<F>(f), 
                        std::forward<Args>(args)...
                    ));
    }

    void wait(){
        for (auto& fut : futures_){
            fut.get();
        }

        futures_.clear();
    }

};