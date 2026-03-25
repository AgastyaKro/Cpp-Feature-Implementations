#include <atomic>
#include <thread>
#include <vector>
#include <cassert>


class SPSCQueue{
private:
    struct alignas(64) PaddedAtomicInt{
        std::atomic<int> index;
    };

    const size_t capacity_;
    const size_t mask_;
    std::vector<int> buffer_;

    PaddedAtomicInt head_;
    PaddedAtomicInt tail_;

    static size_t round_up_to_power_of_two(size_t x){
        size_t power = 1;
        while (power < x){
            power <<= 1;
        }
        return power;
    }

public:

explicit SPSCQueue(size_t requested_capacity) : capacity_(round_up_to_power_of_two(requested_capacity)), mask_(capacity_-1), buffer_((capacity_)){
    assert(capacity_ >= 2);
    head_.index.store(0, std::memory_order_relaxed);
    tail_.index.store(0, std::memory_order_relaxed);
}

bool push(int value){
    int tail = tail_.index.load(std::memory_order_relaxed);
    int next_tail = (tail + 1) & mask_;
    int head = head_.index.load(std::memory_order_acquire);

    if (head == next_tail){
        return false;
    }
    buffer_[tail] = value;
    tail_.index.store(next_tail, std::memory_order_release);
    return true;
    
}

bool pop(int& value){
    int head = head_.index.load(std::memory_order_relaxed);
    int tail = tail_.index.load(std::memory_order_acquire);

    if (head == tail)
        return false;
    value = buffer_[head];
    head_.index.store((head + 1) & mask_, std::memory_order_release);
}

bool empty() const {
        const size_t head = head_.index.load(std::memory_order_acquire);
        const size_t tail = tail_.index.load(std::memory_order_acquire);
        return head == tail;
    }

    bool full() const {
        const size_t tail = tail_.index.load(std::memory_order_acquire);
        const size_t next_tail = (tail + 1) & mask_;
        const size_t head = head_.index.load(std::memory_order_acquire);
        return next_tail == head;
    }

    size_t capacity() const {
        return capacity_ - 1;
    }





};