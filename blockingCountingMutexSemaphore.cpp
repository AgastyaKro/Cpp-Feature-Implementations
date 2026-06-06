#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <optional>
#include <chrono>

class BlockingCountingMutexSemaphore{
private:
    std::condition_variable cv_;
    std::mutex mtx_;
    std::uint64_t count_;

public:
    explicit BlockingCountingMutexSemaphore(std::uint64_t count) : count_(count){}

    void acquire(){
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(lock, [&]{
            return count_ > 0;
        });

        --count_;
    }


    void release(){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            ++count_; 
        }
        cv_.notify_one();
    }

    bool try_acquire(){
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (count_ == 0)
            return false;

        --count_;
        return true;
    }

    template <typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& timeout){
        std::unique_lock<std::mutex> lock(mtx_);

        const bool acquired = cv_.wait_for(lock, timeout, [&]{
            return count_ > 0;
        });

        if (!acquired){
            return false;
        }

        --count_;
        return true;
    }

    class Permit{
        private:
            BlockingCountingMutexSemaphore* sem_;

            struct AdoptTag {};

            Permit(BlockingCountingMutexSemaphore& sem, AdoptTag) : sem_(&sem) {}

            friend class BlockingCountingMutexSemaphore;

        public:

            ~Permit(){
                if (sem_){
                    sem_->release();
                }
            }

            Permit(const Permit&) = delete;
            Permit& operator=(const Permit&) = delete;

            Permit(Permit&& other) noexcept : sem_(other.sem_){
                other.sem_ = nullptr;
            }

            Permit& operator=(Permit&& other) noexcept{
                if (this != &other){
                    if (sem_){
                        sem_->release();
                    }
                    
                    sem_ = other.sem_;
                    other.sem_ = nullptr;
                }
                return *this;

            }


    };

    Permit acquire_guard(){
        acquire();
        return Permit(*this, Permit::AdoptTag{});
    }

    std::optional<Permit> try_acquire_guard(){
        if (!try_acquire()){
            return std::nullopt;
        }
        return Permit(*this, Permit::AdoptTag{});
    }   

    template <typename Rep, typename Period>
    std::optional<Permit> try_acquire_for_guard(const std::chrono::duration<Rep, Period>& timeout){
        if (!try_acquire_for(timeout)){
            return std::nullopt;
        }
        return Permit(*this, Permit::AdoptTag{});
    }

};