#include <atomic>
#include <vector>
#include <thread>

class MPSCQueue{
private:
    struct alignas(64) Cell{
        std::atomic<size_t> sequence;
        int value;
    };
    struct alignas(64) PaddedAtomic{
        std::atomic<size_t> index;
    };

    size_t capacity_;
    size_t mask_;
    std::vector<Cell> buffer_;

    PaddedAtomic enqueue_pos_;
    PaddedAtomic dequeue_pos_;

    static size_t round_up_to_power_of_two(size_t x){
        size_t p = 1;
        while (p < x){
            p <<= 1;
        }
        return p;
    }

public:
    explicit MPSCQueue(size_t requested_capacity) : capacity_(round_up_to_power_of_two(requested_capacity)), mask_(capacity_ - 1), buffer_(capacity_){
        for (int i = 0; i < capacity_; i++){
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }

        enqueue_pos_.index.store(0, std::memory_order_relaxed);
        dequeue_pos_.index.store(0, std::memory_order_relaxed);
    }


    bool push(int value){
        size_t pos = enqueue_pos_.index.load(std::memory_order_relaxed);

        while (true){
            Cell& cell = buffer_[pos & mask_];
            size_t seq = cell.sequence.load(std::memory_order_acquire);

            intptr_t diff = (intptr_t)seq - (intptr_t)pos;

            if (diff == 0){
                if (enqueue_pos_.index.compare_exchange_weak(
                    pos,
                    pos + 1,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed
                )){
                    cell.value = value;
                    cell.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            }
            else if (diff < 0){
                return false; // its full
            }
            else {
                pos = enqueue_pos_.index.load(std::memory_order_relaxed);
            }
        }
    }

    bool pop(int& value){
        size_t pos = dequeue_pos_.index.load(std::memory_order_relaxed);
        Cell& cell = buffer_[pos & mask_];
        size_t seq = cell.sequence.load(std::memory_order_acquire);
            
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);

        if (diff < 0){
            return false;
        }

        value = cell.value;
        dequeue_pos_.index.store(pos + 1, std::memory_order_relaxed);
        cell.sequence.store(pos + capacity_, std::memory_order_release);
        return true;

        
    }

    bool empty() const {
        size_t pos = dequeue_pos_.index.load(std::memory_order_relaxed);
        const Cell& cell = buffer_[pos & mask_];
        size_t seq = cell.sequence.load(std::memory_order_acquire);
        return ((intptr_t)seq - (intptr_t)(pos + 1) < 0);
        // seq - pos + 1 < 0 means its empty
        // seq == pos + 1 means its full

    }





};