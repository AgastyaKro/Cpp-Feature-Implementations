#include <queue>
#include <mutex>
#include <semaphore>

template <typename T>
class BoundedQueue{
private:
    std::queue<T> q;
    std::mutex mtx;

    std::counting_semaphore items; // amount of items
    std::counting_semaphore spaces; // amount of free space

public:

    BoundedQueue(size_t capacity) : items(0), spaces(capacity) {}

    void push(T value){
        spaces.acquire();
        {
            std::lock_gaurd<mutex> lck(mtx);
            q.push(std::move(value));
        }
        items.release();
    }

    T pop(){
        items.acquire();
        T item;
        {
            std::lock_gaurd<mutex> lck(mtx);
            item = q.top();
            q.pop();
        }
        spaces.release();
        return item;
    }

};
