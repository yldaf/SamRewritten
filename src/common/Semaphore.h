#include <mutex>
#include <condition_variable>

/**
 * Why doesn't C++ just provide a semaphore..
 * to keep things more C++esque rather than falling back to POSIX semaphores, just use this:
 * https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
 */
class semaphore
{
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0;

public:
    semaphore(unsigned long count) {
        count_ = count;
    };
    ~semaphore() {};

    void notify() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while(!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }
};