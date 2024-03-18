#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

#include "safe_queue.hpp"

class thread_pool {
 private:
  class thread_worker {  // 内置线程工作类
   private:
    int m_id;             // 工作id
    thread_pool *m_pool;  // 所属线程池
   public:
    thread_worker(thread_pool *pool, const int id) : m_pool(pool), m_id(id) {}

    void operator()() {
      while (!m_pool->m_shutdown) {
        std::optional<std::function<void()>> func;  // 函数对象
        {  // 为线程环境加锁，互访问工作线程的休眠和唤醒
          std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);

          // 如果任务队列为空，阻塞当前线程
          if (m_pool->m_queue.empty()) {
            // 等待条件变量通知，开启线程
            m_pool->m_conditional_lock.wait(lock);  
          }

          // 取出任务队列中的元素
          func = m_pool->m_queue.pop();
        }
        // 如果成功取出，执行工作函数
        if (func.has_value()) {
          func.value()();
        }
      }
    }
  };

  bool m_shutdown;  // 线程池是否关闭

  safe_queue<std::function<void()>> m_queue;  // 执行函数安全队列，即任务队列

  std::vector<std::thread> m_threads;  // 工作线程队列

  std::mutex m_conditional_mutex;  // 线程休眠锁互斥变量

  // 线程环境锁，可以让线程处于休眠或者唤醒状态
  std::condition_variable m_conditional_lock;

 public:
  // 线程池构造函数
  thread_pool(const int n_threads = 4)
      : m_threads(n_threads), m_shutdown(false) {}

  // 删除所有拷贝构造函数和赋值操作符
  thread_pool(const thread_pool &) = delete;
  thread_pool(thread_pool &&) = delete;
  thread_pool &operator=(const thread_pool &) = delete;
  thread_pool &operator=(thread_pool &&) = delete;

  void init() {
    for (int i = 0; i < m_threads.size(); ++i) {
      m_threads.at(i) = std::thread(thread_worker(this, i));  // 分配工作线程
    }
  }

  // 等待线程完成当前任务并关闭线程池
  void shutdown() {
    m_shutdown = true;
    m_conditional_lock.notify_all();  // 通知，唤醒所有工作线程

    for (int i = 0; i < m_threads.size(); ++i) {
      if (m_threads.at(i).joinable()) {  // 判断线程是否在等待
        m_threads.at(i).join();          // 将线程加入到等待队列
      }
    }
  }

  // 提交一个函数到线程池中异步执行
  template <typename F, typename... Args>
  auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    // 通过std::bind函数，将函数和参数绑定，返回一个可调用对象
    using return_type = decltype(f(args...));

    std::function<return_type()> func =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    // 创建一个 shared_ptr<packaged_task> 对象，用于异步获取任务结果
    auto task_ptr =
        std::make_shared<std::packaged_task<return_type()>>(func);

    // 包装任务函数为void函数
    std::function<void()> warpper_func = [task_ptr]() { (*task_ptr)(); };

    // 队列通用安全封包函数，并压入安全队列
    m_queue.push(warpper_func);

    // 唤醒一个等待中的线程
    m_conditional_lock.notify_one();

    // 返回先前注册的任务指针
    return task_ptr->get_future();
  }
};