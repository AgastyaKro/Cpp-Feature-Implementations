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
        std::condition_variable cv;
        bool stop = false;


    public:
        ThreadPool(size_t n){
            for (size_t i = 0; i < n; i++){
                workers.emplace_back([this]{
                    while (true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mtx);

                            cv.wait(lock, [this]{   
                                return stop || !tasks.empty();
                            });
                            if (stop && tasks.empty())
                                return;
                            
                            task = std::move(tasks.front());
                            tasks.pop();

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
        auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result<F, Args...>>{
            using R = invoke_result<F, Args...>;

            auto boundTask = std::bind(
                std::forward<F>(f), std::forward<Args>(args);
            );

            auto task = std::make_shared<std::package_task<R()>>(std::move(boundTask));

            std::future<R> future = task->get_future();

            {
                std::lock_guard<std::mutex> lck(mtx);

                if (stop)
                    throw std::runtime_error("submit on stopped ThreadPool");
                
                task.emplace([task]{
                    (*task)();
            });
        }
        cv.notify_one();

        return future;

        }

        template <class F>
        auto submit(F&& f) -> std::future<std::invoke_result<F>>
        {
        
            using R = decltype(f());
            auto task = std::make_shared<std::packaged_task<R()>>(std::forward(f));

            std::future<R> future = task->get_future();

            {
                std::lock_guard<std::mutex> lck(mtx);

                if (stop)
                    throw std::runtime_error("submit on stopped ThreadPool");

                tasks.emplace([task]{
                    (*task)();
                });
            }
        cv.notify_one();
        return future;
        }

        void shutdown(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                stop = true;
            }
            cv.notify_all();

            for (auto& t : workers){ 
                t.join();
            }
        }





};