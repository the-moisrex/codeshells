#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <cctype>
#include <cstdint>
#include <print>
#include <string_view>
#include <utility>

using namespace std;

namespace mine {
template <std::size_t Len> struct const_string_view : std::array<char, Len> {
  using arr_type = std::array<char, Len>;

  consteval const_string_view(char const (&str)[Len]) noexcept {
    std::copy(str, str + Len, this->begin());
  }

  using arr_type::operator[];
  constexpr std::size_t size() const noexcept { return arr_type::size() - 1; }
};

// template <std::size_t Len>
// const_string_view(char const[Len]) -> const_string_view<Len>;

template <const_string_view Format = "dddd-dd-dd">
constexpr auto for_all(auto func, std::string_view date) noexcept {
  return ([=]<std::size_t... I>(std::index_sequence<I...>) {
    if (date.size() < Format.size()) {
      return false;
    }
    return (func.template operator()<Format[I]>(date[I]) && ...);
  })(std::make_index_sequence<Format.size()>{});
}

constexpr bool verify_date_format(std::string_view date) noexcept {
  return for_all<"dddd-dd-dd">(
      []<char FMChar>(char cur) {
        if constexpr (FMChar == 'd') {
          if (!isdigit(cur)) {
            return false;
          }
        } else if constexpr (FMChar == '-') {
          if (cur != '-') {
            return false;
          }
        } else {
          std::unreachable();
        }
        return true;
      },
      date);
}
} // namespace mine

namespace orig {
bool verify_date_format(const char *date) {
  const char *format = "dddd-dd-dd";
  for (; *format && *date; ++format, ++date) {
    switch (*format) {
    case 'd':
      if (!isdigit(*date))
        return false;
      break;
    case '-':
      if (*date != '-')
        return false;
      break;
    default:
      std::unreachable();
    }
  }
  return !(*format || *date);
}

} // namespace orig

static void Orig(benchmark::State &state) {
  std::string str = "1111-22-33";
  int i = 0;
  for (auto _ : state) {
    ++str[i % str.size()];
    auto res = orig::verify_date_format(str.data());
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Orig);

static void Mine(benchmark::State &state) {
  std::string str = "1111-22-33";
  int i = 0;
  for (auto _ : state) {
    ++str[i % str.size()];
    auto res = mine::verify_date_format({str.data(), str.size()});
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(Mine);
