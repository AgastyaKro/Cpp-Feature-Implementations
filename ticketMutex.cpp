#include <atomic>
#include <immintrin.h>


class TicketMutex{
private:
    std::atomic<unsigned int> next_ticket{0};
    std::atomic<unsigned int> now_serving{0};

public:
    TicketMutex() = default;
    TicketMutex(const TicketMutex&) = delete;
    TicketMutex& operator=(const TicketMutex&) = delete;

    void lock(){
        unsigned int myTicket = next_ticket.fetch_add(1, std::memory_order_relaxed);

        while (now_serving.load(std::memory_order_acquire) != myTicket){
            _mm_pause;
        }

    }

    void unlock(){
        now_serving.fetch_add(1, std::memory_order_relaxed);
    }

};