#include <algorithm>
#include <vector>
#include <thread>
#include <numeric>

#include <gtest/gtest.h>
#include <fmt/core.h>

#include "ThreadPool.hpp"
#include "randint.hpp"
#include "timer.hpp"


// 设置线程睡眠时间
void simulate_hard_computation() {
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// 添加两个数字的简单函数并打印结果
void multiply(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  // fmt::println("{} * {} = {}", a, b, res);
}

// 添加并输出结果
void multiply_output(int &out, const int a, const int b) {
  simulate_hard_computation();
  out = a * b;
  // fmt::println("{} * {} = {}", a, b, out);
}

// 结果返回
int multiply_return(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  // fmt::println("{} * {} = {}", a, b, res);
  return res;
}

void example() {
  // 创建3个线程的线程池
  timer my_timer;
  XF::ThreadPool pool(3);

  // 初始化线程池
  pool.init();

  // 提交乘法操作，总共30个
  for (int i = 1; i <= 3; ++i) {
    for (int j = 1; j <= 10; ++j) {
      pool.submit(multiply, i, j);
    }
  }

  // 使用ref传递的输出参数提交函数
  int output_ref;
  auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

  // 等待乘法输出完成
  future1.get();
  fmt::println("Last operation result is equals to {}", output_ref);

  // 使用return参数提交函数
  auto future2 = pool.submit(multiply_return, 5, 3);

  // 等待乘法输出完成
  int res = future2.get();
  // fmt::println("Last operation result is equals to {}", res);

  // 关闭线程池
  pool.shutdown();
  fmt::println("Elapsed time: {} ms", my_timer.elapsed());
}


TEST(LockfreeCircularQueue, push) {
  XF::Queue::LockfreeCircularQueue<int> queue(10000);
  queue.push(1);
  queue.push(2);
  queue.push(3);
  queue.push(4);
  queue.push(5);
}

TEST(LockfreeCircularQueue, pop) {
  XF::Queue::LockfreeCircularQueue<int> queue(10000);
  std::vector<int> v = {1, 2, 3, 4, 5};
  for (auto i : v) {
    queue.push(std::move(i));
  }

  for (auto i : v) {
    int res = -1;
    bool ok = queue.pop(res);
    ASSERT_TRUE(ok);
    ASSERT_EQ(res, i);
  }
}

TEST(LockfreeCircularQueue, concurrent_push) {
  const int N = 10000;
  const int T = 16;

  XF::Queue::LockfreeCircularQueue<int> queue(N * T);

  std::vector<std::thread> threads;
  for (int i = 0; i < T; ++i) {
    threads.emplace_back([&queue] {
      for (int j = 0; j < N; ++j) {
        queue.push(j);
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  ASSERT_EQ(queue.size(), N * T);

  std::map<int, int> v;
  for (int i = 0; i < N * T; ++i) {
    int res = -1;
    bool ok = queue.pop(res);
    ASSERT_TRUE(ok);
    v[res]++;
  }

  for (int i = 0; i < N; ++i) {
    if (v[i] != T) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_EQ(v[i], T);
  }
}


TEST(LockfreeCircularQueue, concurrent_pop) {
  const int N = 20000;
  const int T = 16;

  XF::Queue::LockfreeCircularQueue<int> queue(N * T);

  std::vector<int> buffer(N * T, -1);

  std::vector<std::thread> threads;
  for (int i = 0; i < T; ++i) {
    if (i % 2 == 0) {
      threads.emplace_back([&queue, &buffer, i] {
        for (int j = 0; j < N; ++j) {
          queue.push(j);
        }
      });
    } else {
      threads.emplace_back([&queue, &buffer, i] {
        for (int j = 0; j < N; ++j) {
          int try_time_limit = 10;
          while (try_time_limit--) {
            int res = -1;
            if (queue.pop(res)) {
              buffer[i * N + j] = res;
              break;
            }

            // wait for a while
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
          if (try_time_limit <= 0) {
            fmt::println("Thread {} pop failed", i);
          }
        }
      });
    }
  }

  for (auto &t : threads) {
    t.join();
  }

  std::map<int, int> v;

  for (int i = 0; i < T; ++i) {
    if (i % 2 == 0) {
      for (int j = 0; j < N; ++j) {
        if (buffer[i * N + j] != -1) {
          fmt::println("buffer[{}][{}] = {}", i, j, buffer[i * N + j]);
        }
        // ASSERT_EQ(buffer[i * N + j], 0);
      }
    } else {
      for (int j = 0; j < N; ++j) {
        v[buffer[i * N + j]]++;
      }
    }
  }

  fmt::println("failed: {}", v[-1]);

  for (int i = 0; i < N; ++i) {
    if (v[i] != T / 2) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_GE(v[i], T / 2 - 10);
  }
}

TEST(MutexQueue, concurrent_pop) {
  const int N = 20000;
  const int T = 16;

  XF::Queue::MutexQueue<int> queue;

  std::vector<int> buffer(N * T, -1);

  std::vector<std::thread> threads;
  for (int i = 0; i < T; ++i) {
    if (i % 2 == 0) {
      threads.emplace_back([&queue, &buffer, i] {
        for (int j = 0; j < N; ++j) {
          queue.push(j);
        }
      });
    } else {
      threads.emplace_back([&queue, &buffer, i] {
        for (int j = 0; j < N; ++j) {
          int try_time_limit = 10;
          while (try_time_limit--) {
            int res = -1;
            bool ok = queue.pop(res);
            if (ok) {
              buffer[i * N + j] = res;
              break;
            }

            // wait for a while
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
          }
          if (try_time_limit <= 0) {
            fmt::println("Thread {} pop failed", i);
          }
        }
      });
    }
  }

  for (auto &t : threads) {
    t.join();
  }

  std::map<int, int> v;

  for (int i = 0; i < T; ++i) {
    if (i % 2 == 0) {
      for (int j = 0; j < N; ++j) {
        if (buffer[i * N + j] != -1) {
          fmt::println("buffer[{}][{}] = {}", i, j, buffer[i * N + j]);
        }
        // ASSERT_EQ(buffer[i * N + j], 0);
      }
    } else {
      for (int j = 0; j < N; ++j) {
        v[buffer[i * N + j]]++;
      }
    }
  }

  ASSERT_EQ(v[-1], 0);

  for (int i = 0; i < N; ++i) {
    if (v[i] != T / 2) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_EQ(v[i], T / 2);
  }
}


TEST(ThreadPool, OK) {
  example();
}


TEST(ThreadPool, MutexQueue) {
  timer my_timer;
  XF::ThreadPool<XF::FMutexQueue> pool(16);
  pool.init();

  for (int i = 1; i <= 10000; ++i) {
    pool.submit(multiply, i, i);
  }

  auto future = pool.submit(multiply_return, 0, 0);

  int val = future.get();
  ASSERT_EQ(val, 0);
  pool.shutdown();
  ASSERT_EQ(pool.size(), 0);
}


TEST(ThreadPool, LockfreeQueue) {
  timer my_timer;
  XF::ThreadPool<XF::FLockfreeQueue> pool(16);

  pool.init();

  for (int i = 1; i <= 10000; ++i) {
    pool.submit(multiply, i, i);
  }

  auto future = pool.submit(multiply_return, 0, 0);

  int val = future.get();
  ASSERT_EQ(val, 0);
  pool.shutdown();
  ASSERT_EQ(pool.size(), 0);
}


// TEST(test_thread_pool, concurrent_push_pop) {
//   timer my_timer;
//   thread_pool<queue::concurrent> pool(10);
//   pool.init();

//   std::vector<int> ans(2000, 0);

//   for (int i = 0; i < 1000; ++i) {
//     pool.submit([]() {
//       int ans = 0;
//       for (int j = 0; j < 1000; ++j) {
//         ans++;
//       }
//     });
//   }

//   pool.shutdown();

//   ASSERT_EQ(pool.size(), 0);
// }