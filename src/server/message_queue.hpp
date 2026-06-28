#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T> struct MessageQueue {
  private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _condition;

  public:
    MessageQueue() = default;
    MessageQueue(const MessageQueue<T> &) = delete;
    MessageQueue &operator=(const MessageQueue<T> &) = delete;
    MessageQueue(MessageQueue<T> &&) noexcept;
    MessageQueue &operator=(MessageQueue<T> &&) noexcept;
    ~MessageQueue() = default;

    size_t send(T &&message);
    T receive();
};

template <typename T>
MessageQueue<T>::MessageQueue(MessageQueue<T> &&other) noexcept {
    std::unique_lock<std::mutex> lock(other._mutex);
    _queue = std::move(other._queue);
}

template <typename T>
MessageQueue<T> &MessageQueue<T>::operator=(MessageQueue<T> &&other) noexcept {
    std::unique_lock<std::mutex> lock(other._mutex);
    _queue = std::move(other._queue);
    return *this;
}

template <typename T> size_t MessageQueue<T>::send(T &&message) {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::forward<T>(message));
    _condition.notify_one();
    return _queue.size();
}

template <typename T> T MessageQueue<T>::receive() {
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });
    T message = std::move(_queue.front());
    _queue.pop();
    return message;
}