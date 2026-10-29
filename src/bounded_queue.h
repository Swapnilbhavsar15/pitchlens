#pragma once
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

// Thread-safe bounded queue. push() blocks when full (back-pressure),
// pop() blocks when empty, and returns nullopt once closed and drained.
template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(size_t capacity) : capacity_(capacity) {}

    bool push(T item) {
        std::unique_lock<std::mutex> lk(m_);
        not_full_.wait(lk, [&] { return q_.size() < capacity_ || closed_; });
        if (closed_) return false;
        q_.push(std::move(item));
        not_empty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lk(m_);
        not_empty_.wait(lk, [&] { return !q_.empty() || closed_; });
        if (q_.empty()) return std::nullopt;
        T item = std::move(q_.front());
        q_.pop();
        not_full_.notify_one();
        return item;
    }

    void close() {
        std::lock_guard<std::mutex> lk(m_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    size_t capacity_;
    std::queue<T> q_;
    std::mutex m_;
    std::condition_variable not_full_, not_empty_;
    bool closed_ = false;
};
