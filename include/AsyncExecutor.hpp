#pragma once

#include "BlockingThreadPool.hpp"

#include <cstddef>
#include <future>
#include <type_traits>
#include <utility>


class AsyncExecutor {

private:
    BlockingThreadPool pool_;

public:

    explicit AsyncExecutor(std::size_t workers, std::size_t queueCapacity) : pool_(workers, queueCapacity) {}

    AsyncExecutor(const AsyncExecutor&) = delete;
    AsyncExecutor(AsyncExecutor&&) = delete;
    AsyncExecutor& operator=(const AsyncExecutor&) = delete;
    AsyncExecutor& operator=(AsyncExecutor&&) = delete;

    template<typename F, typename... Args>
    auto async(F&& f, Args&&... args) -> std::future<std::invoke_result<F, Args...>>{
        return pool_.submit(std::forward<F>(f), std::forward<Args>(args)...);
    }

    void shutdown(){
        pool_.shutdown();
    }

};