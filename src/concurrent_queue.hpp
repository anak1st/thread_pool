#include <memory>
#include <atomic>
#include <fmt/core.h>

template <typename T>
struct node {
  std::shared_ptr<T> data = nullptr;
  std::shared_ptr<node<T>> next = nullptr;
};


template <typename T>
class concurrent_queue {
 private:
  std::atomic_int32_t m_size;

  std::atomic<std::shared_ptr<node<T>>> head;
  std::atomic<std::shared_ptr<node<T>>> tail;

 public:
  concurrent_queue() : m_size(0) {
    std::shared_ptr<node<T>> p = std::make_shared<node<T>>();
    head.store(p);
    tail.store(p);
  }

  template <typename U>
  void push(U &&new_value) {
    std::shared_ptr<node<T>> t = nullptr;
    while (true) {
      if (t == nullptr) {
        t = tail.load();
        continue;
      }
      if (!tail.compare_exchange_weak(t, nullptr)) {
        continue;
      }
      break;
    }

    if (t == nullptr) {
      fmt::println("error: tail is nullptr");
      return;
    }

    std::shared_ptr<node<T>> p = std::make_shared<node<T>>();
    p->data = std::make_shared<T>(std::move(new_value));

    t->next = p;
    tail = p;
    ++m_size;
  }

  std::shared_ptr<T> pop() {
    std::shared_ptr<node<T>> h = nullptr;
    std::shared_ptr<node<T>> h_next = nullptr;

    while (true) {
      if (h == nullptr) {
        h = head.load();
        continue;
      }
      if (!head.compare_exchange_weak(h, nullptr)) {
        continue;
      }
      break;
    }

    if (h == nullptr) {
      fmt::println("error: head is nullptr");
      return nullptr;
    }

    std::shared_ptr<node<T>> p = h->next;
    std::shared_ptr<T> res = nullptr;
    if (p) {
      res = p->data;
      --m_size;
      head = p;
    } else {
      head = h;
    }
    
    return res;
  }

  size_t size() const { return m_size.load(); }

  bool empty() const { return m_size == 0; }
};