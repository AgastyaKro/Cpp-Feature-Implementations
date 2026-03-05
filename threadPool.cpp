#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

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
                            std::unique_lock<std::mutex> lock(mtx); // why unique

                            cv.wait(lock, [this]{   
                                return stop || !tasks.empty();
                            });
                            // whats the point of the wait if we check right after too
                            if (stop && tasks.empty())
                                return;
                            
                            task = std::move(tasks.front());
                            tasks.pop();

                        }   
                        task();
                    }});
                
            }
        }

        ~ThreadPool(){
            shutdown();
        }

        void submit(std::function<void()> task){
            {
                std::lock_guard<std::mutex> lock(mtx); // why not unique lock here?
                tasks.push(std::move(task));
            }
            cv.notify_one();
        }

        void shutdown(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                stop = true;
            }
            cv.notify_all();

            for (auto& t : workers){ // why join all here, is this better in the destructor ro something if an exception is thrown
                t.join();
            }
        }





};