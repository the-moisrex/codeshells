#include <benchmark/benchmark.h>
#include <initializer_list>
#include <cstddef>
#include <string>
#include <memory_resource>
#include <list>

// #include "stack_allocator.h"

using namespace std;

constexpr auto nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

static void EmptyRandom(benchmark::State& state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);

static void STD03(benchmark::State& state) {
  for (auto _ : state) {
    list<int> vecs {nums};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(STD03);


// static void STD03Opted(benchmark::State& state) {
//   const static std::size_t stack_size = 20;
//   int buffer[stack_size];
// 
//   typedef stack_allocator<int, stack_size> allocator_type;
//   std::allocator<int> int_alloc;
//   for (auto _ : state) {
//     allocator_type mr(buffer, &int_alloc);
//     list<int, allocator_type> vecs {nums, mr};
//     benchmark::DoNotOptimize(vecs);
//   }
// }
// BENCHMARK(STD03Opted);



static void Monotonic(benchmark::State& state) {
  for (auto _ : state) {
    byte buff[2048];
    pmr::monotonic_buffer_resource mr(buff, sizeof buff);
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(Monotonic);

static void Sync(benchmark::State& state) {
  for (auto _ : state) {
    pmr::synchronized_pool_resource mr;
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(Sync);

static void USync(benchmark::State& state) {
  for (auto _ : state) {
    pmr::unsynchronized_pool_resource mr;
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(USync);


static void MonotonicHeap(benchmark::State& state) {
  for (auto _ : state) {
    byte* buff = new byte[2048];
    pmr::monotonic_buffer_resource mr(buff, 1);
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MonotonicHeap);


static void MonotonicNoAlloc(benchmark::State& state) {
  byte buff[2048];
  for (auto _ : state) {
    pmr::monotonic_buffer_resource mr(buff, sizeof buff);
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MonotonicNoAlloc);

static void MonotonicHeapNoAlloc(benchmark::State& state) {
  byte* buff = new byte[2048];
  for (auto _ : state) {
    pmr::monotonic_buffer_resource mr(buff, 1);
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MonotonicHeapNoAlloc);



static void MonotonicSameMR(benchmark::State& state) {
  byte buff[2048];
  pmr::monotonic_buffer_resource mr(buff, sizeof buff);
  for (auto _ : state) {
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MonotonicSameMR);

static void SyncSame(benchmark::State& state) {
  pmr::synchronized_pool_resource mr;
  for (auto _ : state) {
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(SyncSame);

static void UnSyncSame(benchmark::State& state) {
  pmr::unsynchronized_pool_resource mr;
  for (auto _ : state) {
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(UnSyncSame);

static void MonotonicHeapSameMR(benchmark::State& state) {
  byte* buff = new byte[2048];
  pmr::monotonic_buffer_resource mr(buff, 1);
  for (auto _ : state) {
    pmr::list<int> vecs {nums, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MonotonicHeapSameMR);


//////////////////////////////// Strings /////////////////////////////////

#define strs {\
  "one",\
  "long",\
  "very very very very long",\
  "looooooooooooooooooooooooooooooonger than you expected",\
  "EEEEEEEEEEEEEEEEEEvvvvvvvvvvvvvveeeeeeeeeeeeeeennnnnnnnnnnnn Looooooooooooonnnnnnnnnnnnnngggggggggggggggggeeeeeeeeeeeeeerrrrrrrrrrrrrrrr",\
  "some",\
  "other",\
  "small",\
  "strings",\
  "as",\
  "well",\
  "this",\
  "should",\
  "be",\
  "enough",\
  "i",\
  "think",\
  "But",\
  "I",\
  "Think",\
  "There",\
  "Should",\
  "Be", "a", "lot", "more", "of", "these", "Don't", "you",\
  "agree", "with", "me",\
  "I got", "bored", "of", "adding", "one", "of", "these", "at",\
  "each", "line", "so", "I'm", "doing", "it", "like", "this", "."\
}

static void STD03Str(benchmark::State& state) {
  for (auto _ : state) {
    list<string> vecs {strs};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(STD03Str);



static void MStr(benchmark::State& state) {
  for (auto _ : state) {
    byte buff[2048];
    pmr::monotonic_buffer_resource mr(buff, sizeof buff);
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MStr);

static void MStrNoAlloc(benchmark::State& state) {
  byte buff[2048];
  pmr::monotonic_buffer_resource mr(buff, sizeof buff);
  for (auto _ : state) {
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(MStrNoAlloc);



static void NewDelStr(benchmark::State& state) {
  for (auto _ : state) {
    pmr::list<pmr::string> vecs {strs, pmr::new_delete_resource()};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(NewDelStr);

///////////////////// pool /////////////////////


static void UPoolStr(benchmark::State& state) {
  for (auto _ : state) {
    pmr::unsynchronized_pool_resource mr{};
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(UPoolStr);


static void PoolStr(benchmark::State& state) {
  for (auto _ : state) {
    pmr::synchronized_pool_resource mr{};
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(PoolStr);

static void UPoolStrNoAlloc(benchmark::State& state) {
  pmr::unsynchronized_pool_resource mr{};
  for (auto _ : state) {
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(UPoolStrNoAlloc);


static void PoolStrNoAlloc(benchmark::State& state) {
  pmr::synchronized_pool_resource mr{};
  for (auto _ : state) {
    pmr::list<pmr::string> vecs {strs, &mr};
    benchmark::DoNotOptimize(vecs);
  }
}
BENCHMARK(PoolStrNoAlloc);



