#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <iostream>


class ThreadPool{
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex mtx;
        std::condition_variable not_empty;
        std::condition_variable not_full;
        bool stop = false;
        size_t maxTasks;


    public:
        ThreadPool(size_t n, size_t maxTasks_) : maxTasks(maxTasks_){
            for (size_t i = 0; i < n; i++){
                workers.emplace_back([this]{
                    while (true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mtx);

                            not_empty.wait(lock, [this]{   
                                return stop || !tasks.empty();
                            });
                            if (stop && tasks.empty())
                                return;
                            
                            task = std::move(tasks.front());
                            tasks.pop();
                            not_full.notify_one();
                        } 
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
                    return stop || tasks.size() < maxTasks;
                });

                if (stop)
                    throw std::runtime_error("submit on stopped ThreadPool");
                
                tasks.emplace([task]{
                    (*task)();

                });
             not_empty.notify_one();
            }
        return future;

    }

        void shutdown(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                stop = true;
            }
            not_empty.notify_all();
            not_full.notify_all();

            for (auto& t : workers){ 
                t.join();
            }
        }





};