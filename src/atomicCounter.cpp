#include <cstdint>
#include <atomic>

class AtomicCounter{
private:
    std::atomic<std::uint64_t> value_{0};

public:

    AtomicCounter() = default;

    explicit AtomicCounter(std::uint64_t initial) : value_(initial){}

    std::uint64_t load() const{
        return value_.load(std::memory_order_relaxed);
    }

    void store(std::uint64_t val){
        value_.store(val, std::memory_order_relaxed);
    }

    std::uint64_t getAndIncrement(){
        return value_.fetch_add(1, std::memory_order_relaxed);
    }

    std::uint64_t incrementAndGet(){
        return value_.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    std::uint64_t getAndDecrement(){
        return value_.fetch_sub(1, std::memory_order_relaxed);
    }

    std::uint64_t decrementAndGet(){
        return value_.fetch_sub(1, std::memory_order_relaxed) - 1;
    }


};