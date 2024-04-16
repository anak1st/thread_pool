#include <algorithm>
#include <vector>
#include <thread>
#include <numeric>

#include <gtest/gtest.h>
#include <fmt/core.h>

#include "thread_pool.hpp"
#include "randint.hpp"
#include "timer.hpp"


auto rnd = randint<int>(500, 600);

// 设置线程睡眠时间
void simulate_hard_computation() {
  // std::this_thread::sleep_for(std::chrono::milliseconds(rnd()));
}

// 添加两个数字的简单函数并打印结果
void multiply(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  fmt::println("{} * {} = {}", a, b, res);
}

// 添加并输出结果
void multiply_output(int &out, const int a, const int b) {
  simulate_hard_computation();
  out = a * b;
  fmt::println("{} * {} = {}", a, b, out);
}

// 结果返回
int multiply_return(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  fmt::println("{} * {} = {}", a, b, res);
  return res;
}

// void example() {
//   // 创建3个线程的线程池
//   timer my_timer;
//   thread_pool pool(3);

//   // 初始化线程池
//   pool.init();

//   // 提交乘法操作，总共30个
//   for (int i = 1; i <= 30; ++i) {
//     for (int j = 1; j <= 100; ++j) {
//       pool.submit(multiply, i, j);
//     }
//   }

//   // 使用ref传递的输出参数提交函数
//   int output_ref;
//   auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

//   // 等待乘法输出完成
//   future1.get();
//   fmt::println("Last operation result is equals to {}", output_ref);

//   // 使用return参数提交函数
//   auto future2 = pool.submit(multiply_return, 5, 3);

//   // 等待乘法输出完成
//   int res = future2.get();
//   fmt::println("Last operation result is equals to {}", res);

//   // 关闭线程池
//   pool.shutdown();
//   fmt::println("Elapsed time: {} ms", my_timer.elapsed());
// }


TEST(test_concurrent_queue, push) {
  concurrent_queue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  queue.push(4);
  queue.push(5);
}

TEST(test_concurrent_queue, pop) {
  concurrent_queue<int> queue;
  std::vector<int> v = {1, 2, 3, 4, 5};
  for (auto i : v) {
    queue.push(std::move(i));
  }

  ASSERT_EQ(queue.size(), v.size());

  for (auto i : v) {
    auto res = queue.pop();
    ASSERT_EQ(*res, i);
  }

  ASSERT_EQ(queue.size(), 0);
}

TEST(test_concurrent_queue, size) {
  concurrent_queue<int> queue;
  std::vector<int> v = {1, 2, 3, 4, 5};
  for (auto i : v) {
    queue.push(std::move(i));
  }

  ASSERT_EQ(queue.size(), v.size());
}

TEST(test_concurrent_queue, concurrent_push) {
  concurrent_queue<int> queue;

  const int N = 10000;
  const int T = 16;

  std::vector<std::thread> threads;
  for (int i = 0; i < T; ++i) {
    threads.emplace_back([&queue] {
      for (int j = 0; j < N; ++j) {
        queue.push(j);
      }
    });
  }

  // fmt::println("if no following message, the program crashed!");

  for (auto &t : threads) {
    t.join();
  }

  ASSERT_EQ(queue.size(), N * T);

  std::map<int, int> v;
  while (!queue.empty()) {
    auto res = queue.pop();
    ASSERT_TRUE(res);
    v[*res]++;
  }

  for (int i = 0; i < N; ++i) {
    if (v[i] != T) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_EQ(v[i], T);
  }
}


TEST(test_concurrent_queue, concurrent_pop) {
  timer my_timer;
  concurrent_queue<int> queue;

  const int N = 10000;
  const int T = 16;

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < T; ++j) {
      queue.push(i);
    }
  }

  ASSERT_EQ(queue.size(), N * T);

  std::vector<int> buffer(N * T, 0);

  std::vector<std::thread> threads;
  for (int i = 0; i < T; ++i) {
    threads.emplace_back([&queue, &buffer, i] {
      for (int j = 0; j < N; ++j) {
        auto res = queue.pop();
        if (!res) {
          res = queue.pop();
        }
        if (!res) {
          res = queue.pop();
        }
        if (!res) {
          res = queue.pop();
        }
        buffer[i * N + j] = *res;
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  fmt::println("if no following message, the program crashed!");
  
  ASSERT_EQ(queue.size(), 0);

  std::map<int, int> v;
  for (int i = 0; i < N * T; ++i) {
    v[buffer[i]]++;
  }

  for (int i = 0; i < N; ++i) {
    if (v[i] != T) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_EQ(v[i], T);
  }
}


TEST(test_concurrent_queue, concurrent_push_pop) {
  timer my_timer;
  concurrent_queue<int> queue;

  const int N = 10000;
  const int T = 16;

  std::vector<int> buffer(N * T, 0);

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
          auto res = queue.pop();
          buffer[i * N + j] = *res;
        }
      });
    }
  }

  for (auto &t : threads) {
    t.join();
  }

  ASSERT_EQ(queue.size(), 0);

  std::map<int, int> v;

  for (int i = 0; i < N * T; ++i) {
    v[buffer[i]]++;
  }

  for (int i = 0; i < N; ++i) {
    if (v[i] != T) {
      fmt::println("v[{}] = {}", i, v[i]);
    }
    ASSERT_EQ(v[i], T);
  }
}


// TEST(test_thread_pool, safe_queue) {
//   using T = thread_pool<queue::safe>;
//   timer my_timer;
//   T pool(10);

//   pool.init();

//   std::vector<int> ans(1000, 0);

//   for (int i = 0; i < 1000; ++i) {
//     pool.submit([&ans, i] {
//       for (int j = 0; j < 10; ++j) {
//         ans[i]++;
//       }
//     });
//   }

//   pool.shutdown();
//   pool.wait();

//   ASSERT_EQ(pool.size(), 0);
// }


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