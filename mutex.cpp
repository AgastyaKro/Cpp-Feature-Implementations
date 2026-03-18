#include <atomic>
#include <thread>
#include <cassert>


class Mutex {
    private:
        std::atomic<bool> locked{false};
        std::thread::id owner;   

    public:
        Mutex() = default;
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;

        void lock(){
            while (true){
                bool expected = false;

                if (locked.compare_exchange_weak(expected, true,std::memory_order_acquire, std::memory_order_relaxed)){
                    return;
                }

                while (locked.load(std::memory_order_relaxed)){
                    std::this_thread::yield();
                }
            }
        }

        bool try_lock(){
            bool expected = false;

            if (locked.compare_exchange_weak(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                return;
        }

        void unlock(){
            assert(owner == std::this_thread::get_id());
            owner = std::thread::id{};
            locked.store(false, std::memory_order_release);

        }

};