#include <benchmark/benchmark.h>
#include <thread>
#include <future>
#include <pthread.h>
#include <sys/types.h> /* pid_t */
#include <unistd.h>  /* _exit, fork */

using namespace std;

static void EmptyRandom(benchmark::State& state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(c);
  }
}
BENCHMARK(EmptyRandom);


static void Thread(benchmark::State& state) {
  for (auto _ : state) {
	  int i = 0;
	  std::thread([&]() {
			  i++;
			  }).join();
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(Thread);







static void Async(benchmark::State& state) {
  for (auto _ : state) {
	  int i = 0;
	  auto a = std::async(std::launch::async, [&] () {
			i++;
			  });
	  a.get();
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(Async);




void* nothing(void *i) {
	auto ii = (int*)i;
	(*ii)++;
	return ii;
}

static void PThread(benchmark::State& state) {
  for (auto _ : state) {
	  int i = 0;
	  pthread_t tr;
		auto a = pthread_create(&tr, NULL, &nothing, (void*)&i);
		if (a)
			pthread_join(tr, NULL);
    benchmark::DoNotOptimize(a);
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(PThread);






static void Forking(benchmark::State& state) {
  for (auto _ : state) {
	  int i = 0;
	  auto pid = fork();
	  if (pid == 0) {
		// child process
		i++;
		_exit(0);
	  } else {
		// parent process

	  }


    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(Forking);

