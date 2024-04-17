#pragma once

#include <queue>
#include <shared_mutex>

namespace XF {
namespace Queue {

template <typename T>
class MutexQueue {
 public:
  MutexQueue() = default;
  MutexQueue(const MutexQueue<T> &) = delete;
  MutexQueue &operator=(const MutexQueue<T> &) = delete;

  auto empty() -> bool const {
    std::shared_lock<std::shared_mutex> lock(m);
    return q.empty();
  }

  auto size() -> size_t const {
    std::shared_lock<std::shared_mutex> lock(m);
    return q.size();
  }

  void push(const T &value) {
    std::unique_lock<std::shared_mutex> lock(m);
    q.push(value);
  }

  // 从队列中取出元素，如果队列为空，返回 nullptr
  auto pop(T &v) -> bool {
    if (q.empty()) {
      return false;
    }
    std::unique_lock<std::shared_mutex> lock(m);
    v = std::move(q.front());
    q.pop();
    return true;
  }

 private:
  std::shared_mutex m;  // 读写锁，相比 std::mutex，支持多个线程同时读
  std::queue<T> q;
};

}  // namespace Queue
}  // namespace XF
