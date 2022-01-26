#include <benchmark/benchmark.h>
#include <string>
#include <charconv>
#include <string_view>
#include <array>

using namespace std;

template <typename INT>
constexpr auto digit_count() noexcept {
  uint_fast8_t t = 0;
  INT a = std::numeric_limits<int>::max();
  while (a /= 10) ++t;
  return t;
}

template <typename CharT = char, typename INT>
constexpr auto to_str(INT i) noexcept {
  std::array<CharT, digit_count<INT>() + 1> str;
  if (auto [p, err] = std::to_chars(str.data(), str.data() + str.size(), i);
      err == std::errc()) {
    return std::string{
        str.data(), static_cast<std::string::size_type>(p - str.data())};

  }
  return std::string();
}




template <typename CharT = char, typename INT>
constexpr auto to_str2(INT i) noexcept {
    constexpr auto _size = digit_count<INT>() + 1;
  CharT str[_size];
  if (auto [p, err] = std::to_chars(str, str + _size, i);
      err == std::errc()) {
    return std::string{
        str, static_cast<std::string::size_type>(p - str)};
  }
  return std::string();
}



template <typename CharT = char, typename INT>
constexpr auto to_str3(INT i) noexcept {
  constexpr auto _size = digit_count<INT>() + 1;
  CharT str[_size] = {};
  std::to_chars(str, str + _size, i);
  return str;
}


    template <typename CharT = char, typename INT>
    constexpr auto to_str4(INT i) noexcept {
        constexpr auto _size = digit_count<INT>() + 1;
        CharT str[_size];
        auto [p, _] = std::to_chars(str, str + _size, i);
        *p = '\0';
        return std::string{str};
    }

    template <typename CharT = char, typename INT>
    constexpr auto to_str5(INT i) noexcept {
        constexpr auto _size = digit_count<INT>() + 1;
        CharT str[_size];
        auto [p, _] = std::to_chars(str, str + _size, i);
        return std::string{str, static_cast<std::string::size_type>(p - str)};
    }



    template <typename INT, typename CharT = char,
              typename CharTraits = std::char_traits<CharT>, typename... R>
    constexpr auto to_str6(INT value, R&&... args) noexcept {
        constexpr auto _size = digit_count<INT>() + 1;
        CharT          str[_size];
        auto [p, _] =
          std::to_chars(str, str + _size, value, std::forward<R>(args)...);
        return std::string{str, static_cast<std::string::size_type>(p - str)};
    }




////////////////////////////////////////////////////////////
static void ToString(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_string(i++));
  }
}
BENCHMARK(ToString);



static void ToChars(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    std::array<char, 10> data;
    if (auto [p, err] = to_chars(data.data(), data.data() + data.size(), i++);
        err == std::errc() ) {
      benchmark::DoNotOptimize(std::string_view(data.data(), p - data.data()));
    }
    benchmark::DoNotOptimize(std::string_view());
  }
}
BENCHMARK(ToChars);




static void MyVersion(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str(i++));
  }
}
BENCHMARK(MyVersion);



static void MyVersion2(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str2(i++));
  }
}
BENCHMARK(MyVersion2);



static void MyVersion3(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str3(i++));
  }
}
BENCHMARK(MyVersion3);




static void MyVersion4(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str4(i++));
  }
}
BENCHMARK(MyVersion4);



static void MyVersion5(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str5(i++));
  }
}
BENCHMARK(MyVersion5);



static void MyVersion6(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str6(i++));
  }
}
BENCHMARK(MyVersion6);




    template <typename INT, typename CharT = char,
      typename CharTraits = std::char_traits<CharT>, typename... R>
    constexpr auto to_str7(INT value, R&&... args) noexcept {
        constexpr std::string::size_type _size = digit_count<INT>() + 1;
        std::string str(_size, '\0');
        std::to_chars(str.data(), str.data() + _size, value, std::forward<R>(args)...);

        return str;

    }



static void MyVersion7(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str7(i++));
  }
}
BENCHMARK(MyVersion7);







//////////////////////////////////////////////////////////
//
//
//
//




    template <typename CharT>

    struct basic_std_traits {

        using char_type   = CharT;

        using char_traits = std::char_traits<char_type>;



        template <typename Type>

        using allocator = std::allocator<Type>;



        using string_view_type = std::basic_string_view<char_type, char_traits>;

        using string_type =

          std::basic_string<char_type, char_traits, allocator<char_type>>;



        using stringstream_type =

          std::basic_stringstream<char_type, char_traits, allocator<char_type>>;

    };



    using std_traits = basic_std_traits<char>;





    /**

     * A series of simple tests to check if the specified template parameter

     * Traits is a valid traits or not.

     *

     * This set of tests are not yet fool proof, but they hold the fort until

     * later.

     *

     * todo: add more test and make them more clever than this

     */

    template <typename Traits>

    using is_traits = std::conjunction<

      std::is_integral<typename Traits::char_type>,

      std::is_integral<typename Traits::char_traits::char_type>>;



    template <typename Traits>

    constexpr bool is_traits_v = is_traits<Traits>::value;





    /**

     * Automatically choose a string type based on mutability requested

     */

    template <typename Traits, bool Mutable>

    using auto_string_type =

      ::std::conditional_t<Mutable, typename Traits::string_type,

                           typename Traits::string_view_type>;









    template <typename Traits = std_traits, typename ValueType, typename... R>

    constexpr std::enable_if_t<is_traits_v<Traits>, typename Traits::string_type> to_str8(ValueType value, R&&... args) noexcept {

        using char_type = typename Traits::char_type;

        using str_t = typename Traits::string_type;

        using size_type = typename str_t::size_type;

        constexpr size_type _size =

          digit_count<ValueType>() + 1;

        if constexpr (std::is_same_v<char_type, char>) {

            typename Traits::string_type str(_size, '\0');

            std::to_chars(str.data(), str.data() + _size, value,

                          std::forward<R>(args)...);

            return str;

        } else {

            char str[_size];

            auto [p, _] = std::to_chars(str, str + _size, value,

                          std::forward<R>(args)...);

            size_type __size = static_cast<size_type>(p - str);

            str_t res(__size, '\0');

             auto it = res.begin();

            for (auto _c = str; *_c; ++_c) {

                *it++ = static_cast<char_type>(*_c);

            }



            return res;

        }

    }








static void MyVersion8(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_str8(i++));
  }
}
BENCHMARK(MyVersion8);

