# thread_pool

## 所用技术
- shared_mutex 
- mutex, condition_variable
- future, packaged_task
- atomic, lockfree queue

###
- 互斥锁队列
- 无锁队列，使用CAS操作，实现无锁（循环）队列，性能提升大约10%


## 使用方法
```cpp
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
  XF::ThreadPool<XF::FMutexQueue> pool(16);     // 使用带互斥锁的队列
  XF::ThreadPool<XF::FLockfreeQueue> pool(16);  // 使用无锁队列

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

```
