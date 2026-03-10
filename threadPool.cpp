#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <iostream>


class ThreadPool{
    private:
        std::vector<std::thread> workers;

        using Task = std::function<void()>;

        std::vector<Task> buffer;
        size_t head = 0;
        size_t tail = 0;
        size_t capacity;
        size_t count = 0;
        

        std::mutex mtx;
        std::condition_variable not_empty;
        std::condition_variable not_full;
        bool stop = false;


    public:
        ThreadPool(size_t n, size_t maxTasks_) : capacity(maxTasks_), buffer(maxTasks_){

            if (n == 0)
                throw std::invalid_argument("ThreadPool must have at least one worker");
            if (capacity == 0)
                throw std::invalid_argument("maxTasks must be > 0");

            for (size_t i = 0; i < n; i++){
                workers.emplace_back([this]{
                    while (true){
                        Task task;
                        {
                            std::unique_lock<std::mutex> lock(mtx);

                            not_empty.wait(lock, [this]{   
                                return stop || count > 0;
                            });

                            if (stop && count == 0)
                                return;
                            
                            task = std::move(buffer[head]);
                            head = (head + 1) % capacity;
                            --count;
                        } 
                        not_full.notify_one();

                        try{
                            task();
                        }  
                        catch (...){
                            std::cerr << "task threw exception\n";
                        }
                    }});
            }
        }

        ~ThreadPool(){
            shutdown();
        }

        template<class F, class... Args>
        auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>{
            using R = std::invoke_result_t<F, Args...>;

            auto boundTask = std::bind(
                std::forward<F>(f), std::forward<Args>(args)...
            );

            auto task = std::make_shared<std::packaged_task<R()>>(std::move(boundTask));

            std::future<R> future = task->get_future();

            {
                std::unique_lock<std::mutex> lck(mtx);

                not_full.wait(lck, [this]{
                    return stop || count < capacity;
                });

                if (stop)
                    throw std::runtime_error("submit on stopped ThreadPool");
                
                buffer[tail] = [task]{
                    (*task)();
                };
                tail = (tail + 1) % capacity;
                ++count;
            }
        not_empty.notify_one();
        return future;
    }

        void shutdown(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (stop)
                    return;
                stop = true;
            }
            not_empty.notify_all();
            not_full.notify_all();

            for (auto& t : workers){ 
                if (t.joinable()){
                    if (t.get_id() == std::this_thread::get_id()){
                        continue;
                    }
                }
                t.join();
            }
        }

};