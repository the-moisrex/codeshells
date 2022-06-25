
template <template <typename> typename T>
struct one {
  using type = T<int>;
};

template <typename...T>
struct two {

};

auto main() -> int {

  using type = one<two>;

  return 0;
}
