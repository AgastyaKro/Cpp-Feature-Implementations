#include <queue>
#include <mutex>
#include <semaphore>
#include <climits>

template <typename T>
class BoundedQueue{
private:
    std::queue<T> q;
    std::mutex mtx;

    std::counting_semaphore<INT_MAX> items; // amount of items
    std::counting_semaphore<INT_MAX> spaces; // amount of free space

public:

    BoundedQueue(size_t capacity) : items(0), spaces(capacity) {}

    void push(T value){
        spaces.acquire();
        {
            std::lock_guard<std::mutex> lck(mtx);
            q.push(std::move(value));
        }
        items.release();
    }

    T pop(){
        items.acquire();
        T item;
        {
            std::lock_guard<std::mutex> lck(mtx);
            item = std::move(q.front());
            q.pop();
        }
        spaces.release();
        return item;
    }

};
