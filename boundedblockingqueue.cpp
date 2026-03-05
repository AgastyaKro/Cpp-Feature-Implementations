#include <queue>
#include <mutex>
#include <semaphore>
#include <climits>
#include <stdexcept>

template <typename T>
class BoundedQueue{
private:
    std::queue<T> q;
    std::mutex mtx;

    std::counting_semaphore<INT_MAX> items; // amount of items
    std::counting_semaphore<INT_MAX> spaces; // amount of free space
    bool closed = false;

public:

    BoundedQueue(size_t capacity) : items(0), spaces(capacity) {}

    void push(T value){
        spaces.acquire();

        try{
            std::lock_guard<std::mutex> lck(mtx);
            if (closed){
                throw std::runtime_error("Queue closed");
            }
            q.push(std::move(value));
        }
        catch (...){
            spaces.release();
            throw;
        }
        items.release();
    }

    T pop(){
        items.acquire();
        T item;
        {
            std::lock_guard<std::mutex> lck(mtx);
            if (q.empty() || closed){
                throw std::runtime_error("Queue closed");
            }
            item = std::move(q.front());
            q.pop();
        }
        spaces.release();
        return item;
    }

    void close(){
        std::lock_guard<std::mutex> lck(mtx);
        {
        closed = true;
        }
        items.release(INT_MAX);
        spaces.release(INT_MAX);
    }


};
