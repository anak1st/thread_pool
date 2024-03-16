#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class safe_queue {
 public:
  safe_queue() = default;
  safe_queue(const safe_queue<T>&) = delete;
  safe_queue& operator=(const safe_queue<T>&) = delete;


  auto empty() -> bool const {
    std::lock_guard<std::mutex> lock(m);
    return q.empty();
  }

  auto size() -> size_t const {
    std::lock_guard<std::mutex> lock(m);
    return q.size();
  }

  void push(const T& value) {
    std::lock_guard<std::mutex> lock(m);
    q.push(value);
  }
  
  // 从队列中取出元素，如果队列为空，返回std::nullopt
  auto pop() -> std::optional<T> {
    std::lock_guard<std::mutex> lock(m);
    if (q.empty()) {
      return std::nullopt;
    }
    T value = std::move(q.front());
    q.pop();
    return value;
  }

 private:
  std::mutex m;
  std::queue<T> q;
};