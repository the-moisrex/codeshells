#include <cassert>
#include <vector>
#include <cstdint>
#include <bit>
#include <iostream>
#include <fmt/core.h>

// https://twitter.com/incomputable/status/1734602135495451059
// https://godbolt.org/z/KsM591avx

template <class iterator, class BinaryPredicate>
iterator unstable_unique(iterator first, iterator last, BinaryPredicate p) {
    if (first == last || std::next(first) == last)
        return last;  // 0 or 1-element range

    const auto real_first = first;
    const auto real_last = last;


    std::cout << "-----------" << std::endl;

    uint64_t cache = 0b10ULL;
    for (auto pos = first + 1; pos != last; ++pos) {
        cache <<= 1;
        auto const cur = *std::prev(pos);
        cache |= p(cur, *pos);
        fmt::print(">> {} <=> {} | {:b}\n", cur, *pos, cache);
        std::cout << std::flush;
    }
    // cache |= 0b1 << (last - first);
    auto const lsize = std::countl_zero(cache);
    auto lcache = (cache << 1 | 0b1) << lsize;
    cache &= ~((~((~0ULL)>>1)) >> lsize); // turning first left 1 off
    fmt::print("LSize: {} {} lcache-{:b} {:b}\n", lsize, 64-lsize, lcache, (~((~0)>>1)) >> lsize);
    std::cout << std::flush;
    int last_i = 0;
    for (;;) {

        auto const rones = std::countr_one(cache);
        first += rones;
        cache >> rones; // turning first right 1 off

        auto const lzeros = std::countl_zero(lcache);
        lcache << lzeros;
        last -= lzeros;
        last_i += lzeros;
        // cache &= ~(0b1 << (63 - lzeros));   // turning left non-duplicate, a duplicate
        
        fmt::print("  l{} r{} (swap: {} {}) {:b}", lzeros, rones, last - real_first, last - real_first, cache);
        fmt::print(" {:b}\n", cache);
        std::cout << std::flush;

        if (first >= last) {
            break;
        }
        *first = std::move(*last);
    }
    fmt::print("End: {} Size: {} Data: {} i:{}\n", 
        last - real_first,
        real_last - real_first,
        *last,
        last_i);
    std::cout << std::flush;
    return last;
}

template <class iterator>
iterator unstable_unique(iterator first, iterator last) {
    return unstable_unique(first, last, [](auto& a, auto& b) { return a == b; });
}

int main() {
    std::vector<int> v1;
    auto new_end1 = unstable_unique(v1.begin(), v1.end());
    assert(v1.end() == new_end1);

    std::vector<int> v2 = { 1, 2, 3, 4, 5 };
    auto new_end2 = unstable_unique(v2.begin(), v2.end());
    // std::cout << new_end2 - v2.begin() << '\n';
    assert(v2.end() == new_end2);
    // std::copy(v2.begin(), new_end2, std::ostream_iterator<int>(std::cout, " "));
    assert(std::vector<int>({ 1, 2, 3, 4, 5 }) == v2);

    std::vector<int> v3 = { 1, 1, 2, 3, 4, 5 };
    auto new_end3 = unstable_unique(v3.begin(), v3.end());
    assert((v3.begin() + 5 == new_end3));
    assert(std::vector<int>({ 1, 5, 2, 3, 4, 5 }) == v3);

    std::vector<int> v4 = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5 };
    auto new_end4 = unstable_unique(v4.begin(), v4.end());
    assert((v4.begin() + 5 == new_end4));
    assert(std::vector<int>({ 1, 5, 2, 4, 3, 3, 4, 4, 5, 5 }) == v4);

    std::vector<int> v5 = { 1, 1, 1, 1, 1 };
    auto new_end5 = unstable_unique(v5.begin(), v5.end());
    assert((v5.begin() + 1 == new_end5));
    assert(std::vector<int>({ 1, 1, 1, 1, 1 }) == v5);

    std::vector<int> v6 = { 1, 1, 1, 1, 1, 2 };
    auto new_end6 = unstable_unique(v6.begin(), v6.end());
    assert((v6.begin() + 2 == new_end6));
    assert(std::vector<int>({ 1, 2, 1, 1, 1, 2 }) == v6);

    std::vector<int> v7 = { 1, 2, 2, 3, 3, 3, 4, 4, 4, 4 };
    auto new_end7 = unstable_unique(v7.begin(), v7.end(), [](int a, int b) { return a == b; });
    assert((v7.begin() + 4 == new_end7));
    assert(std::vector<int>({ 1, 2, 4, 3, 3, 3, 4, 4, 4, 4 }) == v7);
};

