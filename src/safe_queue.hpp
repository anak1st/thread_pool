#include <queue>
#include <shared_mutex>

template <typename T>
class safe_queue {
 public:
  safe_queue() = default;
  safe_queue(const safe_queue<T> &) = delete;
  safe_queue &operator=(const safe_queue<T> &) = delete;

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
  auto pop() -> std::shared_ptr<T> {
    if (q.empty()) {
      return nullptr;
    }
    std::unique_lock<std::shared_mutex> lock(m);
    T value = std::move(q.front());
    q.pop();
    return std::make_shared<T>(value);
  }

 private:
  std::shared_mutex m;  // 读写锁，相比 std::mutex，支持多个线程同时读
  std::queue<T> q;
};