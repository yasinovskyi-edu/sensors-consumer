#include <queue>
#include <condition_variable>
#include <mutex>

template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const T &item)
    {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    bool try_pop(T &item)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty())
        {
            return false;
        }
        item = std::move(queue.front());
        queue.pop();
        return true;
    }

    void wait_and_pop(T &item)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return !queue.empty(); });
        item = std::move(queue.front());
        queue.pop();
    }
};
