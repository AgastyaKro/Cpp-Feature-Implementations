#include <atomic>
#include <array>
#include <optional>
#include <cstddef>

template<typename T, std::size_t Capacity>
class LockFreeSPSCQueue{
    static_assert(Capacity >= 2, "Capacity must be at least 2");
private:
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::atomic<std::size_t> head_{0};
    std::array<T, Capacity> buffer_{};

    static constexpr std::size_t increment(std::size_t index){
        return (index + 1) % Capacity;
    }

public:
    std::optional<T> pop(){
        const std::size_t head = head_.load(std::memory_order_relaxed);

        if (head == tail_.load(std::memory_order_acquire))
            return std::nullopt;
        
        T value = buffer_[head];
        head_.store(increment(head), std::memory_order_release);

        return value;

    }   

    bool push(const T& value){
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t nextTail = increment(tail);

        if (nextTail == head_.load(std::memory_order_acquire)){
            return false;
        }

        buffer_[tail] = value;
        tail_.store(nextTail, std::memory_order_release);

        return true;

    }
};