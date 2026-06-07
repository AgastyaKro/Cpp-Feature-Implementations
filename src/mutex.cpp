#include <atomic>
#include <thread>
#include <cassert>
#include <immintrin.h>


class Mutex {
    private:
        std::atomic<bool> locked{false};

        #ifndef NDEBUG
        std::thread::id owner{};
        #endif

        static void backoff(int& delay){
            for (int i = 0; i < delay; i++){
                _mm_pause();
            }
            if (delay < 1024){
                delay *= 2;
            }
            else {
                std::this_thread::yield();
            }
        }

    public:
        Mutex() = default;
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;

        void lock(){
            int delay = 1;
            while (true){
                while (locked.load(std::memory_order_relaxed)){
                    _mm_pause();
                }

                bool expected = false;
                if (locked.compare_exchange_weak(expected, true,std::memory_order_acquire, std::memory_order_relaxed)){
                    owner = std::this_thread::get_id();
                    return;
                }

                backoff(delay);
            }
        }

        bool try_lock(){
            bool expected = false;

            if (locked.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed)){
                owner = std::this_thread::get_id();
                return true;

            }
            return false;
        }

        void unlock(){
            assert(owner == std::this_thread::get_id());
            owner = std::thread::id{};
            locked.store(false, std::memory_order_release);

        }

};