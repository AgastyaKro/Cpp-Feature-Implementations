#include <atomic>
#include <thread>
#include <vector>
#include <cassert>
#include <iostream>


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
    return true;
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

int main(){
    {
        SPSCQueue q(8);
        assert(q.empty());
        assert(!q.full());
        
        assert(q.push(10));
        assert(q.push(20));
        assert(q.push(30));

        int value = 0;
        assert(q.pop(value) && value == 10);
        assert(q.pop(value) && value == 20);
        assert(q.pop(value) && value == 30);
        assert(!q.pop(value));
        assert(!q.full());
        assert(q.empty());

    }

    {
        constexpr int N = 1'000'000;
        SPSCQueue q(1024);

        std::atomic<bool> producer_done = false;
        std::atomic<long long> sum_consumed = 0;

        std::thread producer([&](){
            for (int i = 1; i <= N; i++){
                while (!q.push(i)){

                }
            }
            producer_done.store(true, std::memory_order_release);
        }
        );

        std::thread consumer([&] (){
            long long local_sum = 0;
            int value = 0;

            while (!producer_done.load(std::memory_order_acquire) || !q.empty()){
                if (q.pop(value)){
                    local_sum += value;
                }

            }
            sum_consumed.store(local_sum, std::memory_order_release);
        });

        producer.join();
        consumer.join();

        const long long expected = 1LL * N * (N+1) / 2;
        const long long got = sum_consumed.load(std::memory_order_acquire);

        std::cout << "Expected sum: " << expected << "\n";
        std::cout << "Consumed sum: " << got << "\n";
        assert(expected == got);
    }

    std::cout << "All tests passed." << "\n";
    return 0;

}