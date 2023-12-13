#include <cassert>
#include <vector>

template <class iterator, class BinaryPredicate>
iterator unstable_unique(iterator first, iterator last, BinaryPredicate p) {
    if (first == last || std::next(first) == last)
        return last;  // 0 or 1-element range

    ++first;  // position first on the first unknown element (first element will stay by definition)
    bool first_is_known_duplicate = false;  // see below for description

    for (; first < last; ++first) {
        if (!first_is_known_duplicate) {
            if (!p(*first, *std::prev(first))) {
                continue;
            }
        }
        // Here we know that `*first` is a dupe and should be replaced. Also the range is not empty.
        assert(first < last);
        for (--last;; --last) {
            if (first == last)
                return first;  // just past the last unique element
            assert(first < last);
            if (!p(*last, *std::prev(last)))
                break;
        }
        assert(!p(*first, *last));
        // Here we know we're good to replace *first with *last.
        // Complicating matter: if we do so, we "forget" whether *std::next(first) is a duplicate of *first.
        // Maintain `first_is_known_duplicate` to keep track of that.
        first_is_known_duplicate = p(*first, *std::next(first));
        *first = std::move(*last);
    }

    return first;
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

