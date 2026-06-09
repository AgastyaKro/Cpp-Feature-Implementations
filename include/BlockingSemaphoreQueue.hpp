#pragma once
#include "BlockingCountingMutexSemaphore.hpp"
#include <cstdint>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <optional>
#include <utility>
#include <chrono>

template <typename T>
class BlockingSemaphoreQueue{
private:
    BlockingCountingMutexSemaphore itemsAvailable_;
    BlockingCountingMutexSemaphore slotsLeft_;
    std::queue<T> q_;
    std::mutex mtx_;
    std::uint64_t capacity_;

public:
    BlockingSemaphoreQueue(std::uint64_t capacity) : itemsAvailable_(0), slotsLeft_(capacity), capacity_(capacity) {
        if (capacity == 0)
            throw std::invalid_argument("Queue capacity can't be 0");
    }

    void push(T value){
        slotsLeft_.acquire();
        try {
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(std::move(value));
        }
        catch (...){
            slotsLeft_.release();
            throw;
        }
        itemsAvailable_.release();
    }

    T pop(){
        itemsAvailable_.acquire();

        T value = [&]
        {
            std::lock_guard<std::mutex> lock(mtx_);

            T valueMoved = std::move(q_.front());
            q_.pop();

            return valueMoved;
        }();
        slotsLeft_.release();
        return value;
    }

    bool try_push(T value){
        if (!slotsLeft_.try_acquire()){
            return false;
        }
        try {
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(std::move(value));
        }
        catch (...){
            slotsLeft.release();
            throw;
        }
        itemsAvailable_.release();
        return true;
    }

    std::optional<T> try_pop(){
        if (!itemsAvailable_.try_acquire()){
            return std::nullopt;
        }
        T value = [&]
        {
            std::lock_guard<std::mutex> lock(mtx_);
            T valueMoved = std::move(q_.front());
            q_.pop();
            return valueMoved;
        }();
        slotsLeft_.release();
        return value;

    }

    template <typename Rep, typename Period>
    bool try_push_for(T value, const std::chrono::duration<Rep, Period>& timeout){
        if (!slotsLeft_.try_acquire_for(timeout))
            return false;

        try {
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(std::move(value));
        }
        catch (...){
            slotsLeft.release();
            throw;
        }
        itemsAvailable_.release();
        return true;
    }

    template<typename Rep, typename Period>
    std::optional<T> try_pop_for(const std::chrono::duration<Rep, Period>& timeout){
        if (!itemsAvailable_.try_acquire_for(timeout))
            return std::nullopt;
        
        T value = [&]{
            std::lock_guard<std::mutex> lock(mtx_);
            T valueMoved = std::move(q_.front());
            q_.pop();
            return valueMoved;
        }();

        slotsLeft_.release();
        return value;

    }
};