#include <chrono>

class timer {
  using clock = std::chrono::high_resolution_clock;

  clock::time_point start;

 public:
  timer() : start(clock::now()) {}

  // returns the elapsed time in milliseconds
  double elapsed() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() -
                                                                 start)
        .count();
  }
};