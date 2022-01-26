#include <benchmark/benchmark.h>
#include </home/moisrex/Projects/webpp/core/include/webpp/utils/strings.hpp>


using namespace std;

constexpr auto headers = R"header(Host
User-Agent
Accept
Accept-Language
Accept-Encoding
Referer
DNT
Connection
TE
)header";

constexpr auto lower_headers = R"header(host
user-agent
accept
accept-language
accept-encoding
referer
dnt
connection
te
)header";

static void WithOptionalCaseSensitive(benchmark::State& state) {
  string _headers = headers;
  string _lower_headers = lower_headers;
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::equal(_headers.begin(), _headers.end(), _lower_headers.begin(), [] (auto &&c, auto &&c2) {
      return std::tolower(c) == std::tolower(c2);
    }));
  }

}
BENCHMARK(WithOptionalCaseSensitive);


// BENCHMARK_MAIN();
