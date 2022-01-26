#include <benchmark/benchmark.h>
#include <thread>
#include <shared_mutex>
#include <mutex>

using namespace std;

static void LockGuard(benchmark::State& state) {
  mutex m;
  for (auto _ : state) {
    lock_guard<mutex> locked(m);
    benchmark::DoNotOptimize(locked);
  }
}
BENCHMARK(LockGuard);


static void ScopedLock(benchmark::State& state) {
  mutex m;
  for (auto _ : state) {
    scoped_lock<mutex> locked(m);
    benchmark::DoNotOptimize(locked);
  }
}
BENCHMARK(ScopedLock);



static void ScopedLockPlusMutex(benchmark::State& state) {
  for (auto _ : state) {
    mutex m;
    scoped_lock<mutex> locked(m);
    benchmark::DoNotOptimize(locked);
  }
}
BENCHMARK(ScopedLockPlusMutex);


static void SharedLock(benchmark::State& state) {
  shared_mutex m;
  for (auto _ : state) {
    shared_lock locked(m);
    benchmark::DoNotOptimize(locked);
  }
}
BENCHMARK(SharedLock);

static void SharedLockRelocking(benchmark::State& state) {
  shared_mutex m;
  shared_lock locked(m, defer_lock);
  for (auto _ : state) {
    locked.lock();
    benchmark::DoNotOptimize(locked);
    locked.unlock();
  }
}
BENCHMARK(SharedLockRelocking);

static void UniqueLock(benchmark::State& state) {
  mutex m;
  for (auto _ : state) {
    unique_lock<mutex> locked(m);
    benchmark::DoNotOptimize(locked);
  }
}
BENCHMARK(UniqueLock);


static void UniqueLockRelocking(benchmark::State& state) {
  mutex m;
  unique_lock<mutex> locked(m, defer_lock);
  for (auto _ : state) {
    locked.lock();
    benchmark::DoNotOptimize(locked);
    locked.unlock();
  }
}
BENCHMARK(UniqueLockRelocking);


static void Empty(benchmark::State& state) {
  mutex m;
  for (auto _ : state) {
    benchmark::DoNotOptimize(m);
  }
}
BENCHMARK(Empty);

// BENCHMARK_MAIN();

