import standard;

#include <cstdint>
#include <iostream>
#include <print>
#include <ranges>
#include <vector>

using namespace std;

template <class T, class A>
class VectorView : public std::ranges::view_interface<VectorView<T, A>> {
  public:
    VectorView() = default;

    VectorView(const std::vector<T, A>& vec)
        : m_begin(vec.cbegin()), m_end(vec.cend()) {}

    auto begin() const { return m_begin; }

    auto end() const { return m_end; }

  private:
    typename std::vector<T, A>::const_iterator m_begin{}, m_end{};
};

int main() {
    std::vector<int> v = {1, 4, 9, 16};

    VectorView view_over_v{v};

    // We can iterate with begin() and end().
    for (int n : view_over_v | views::filter([](auto i) { return i > 4; }))
        std::cout << n << ' ';
    std::cout << '\n';

    // We get operator[] for free when inheriting from view_interface
    // since we satisfy the random_access_range concept.
    for (std::ptrdiff_t i = 0; i != view_over_v.size(); ++i)
        std::cout << "v[" << i << "] = " << view_over_v[i] << '\n';
}
