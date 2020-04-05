#include <benchmark/benchmark.h>
#include <limits>
#include <memory>
#include <vector>

using namespace std;

template <typename T> struct arena_allocator {
  using value_type = T;

  using pointer = T *;
  using const_pointer = const T *;

  using void_pointer = void *;
  using const_void_pointer = const void *;

  using size_type = size_t;

  using difference_type = std::ptrdiff_t;

  template <typename U>
    struct rebind {
        using other = arena_allocator<U>;
    };

  arena_allocator() = default;
  ~arena_allocator() = default;

  template <typename U>
    arena_allocator(const arena_allocator<U> & other) {}

  pointer allocate(size_type size) {
    counter += size;
    return static_cast<pointer>(operator new(sizeof(T) * size));
  }

  pointer allocate(size_type size, const_void_pointer /* hint */) {
    return allocate(size);
  }

  void deallocate(pointer p, size_type /* size */) { operator delete(p); }

  size_type max_size() { return std::numeric_limits<size_type>::max(); }

  template <typename U, typename... Args>
    void construct(U *p, Args &&... args) {
    new (p) U(std::forward<Args>(args)...);
  }

  template <typename U>
    void destroy(U*p) {
      p->~U();
    }

  private:
    static size_type counter;
};

template <typename T>
typename arena_allocator<T>::size_type arena_allocator<T>::counter = 0;


static void Default(benchmark::State &state) {
  vector<int> data;

  int i = 0;
  for (auto _ : state) {
    data.push_back(i++);
    benchmark::DoNotOptimize(data);
  }
}
BENCHMARK(Default);

static void ArenaAllocator(benchmark::State &state) {
  vector<int, arena_allocator<int>> data;

  int i = 0;
  for (auto _ : state) {
    data.push_back(i++);
    benchmark::DoNotOptimize(data);
  }
}
BENCHMARK(ArenaAllocator);

// BENCHMARK_MAIN();
