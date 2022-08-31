#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mtx_;
    std::queue<std::shared_ptr<T>> queue_;
    std::condition_variable queue_cv_;
public:
    threadsafe_queue(){}
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_cv_.wait(lk, [this]{
            return queue_.size();
        });
        value = std::move(*queue_.front());
        queue_.pop();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lg(mtx_);
        if(queue_.empty())
            return false;
        value = std::move(*queue_.front());
        queue_.pop();
        return true;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lg(mtx_);
        queue_.push(std::make_shared<T>(new_value));
        queue_cv_.notify_one();
    }
    bool empty() {
        std::lock_guard<std::mutex> lg(mtx_);
        return queue_.empty();
    }
};