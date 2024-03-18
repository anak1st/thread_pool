#include <random>

template <typename T>
class randint {
  static_assert(std::is_integral<T>::value, "randint requires an integral type");

  public:
    randint(T min, T max) : mt{std::random_device{}()}, dist(min, max) {}

    T operator()() {
      return dist(mt);
    }

  private:
    std::mt19937 mt;
    std::uniform_int_distribution<T> dist;
};