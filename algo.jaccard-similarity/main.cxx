import standard;

#include <cstdint>
#include <print>
#include <span>
#include <unordered_set>
#include <vector>

using namespace std;

template <typename T>
[[nodiscard]] double jaccard_similarity(std::span<T> a, std::span<T> b) {
    const std::unordered_set<T> set_a(a.begin(), a.end());
    const std::unordered_set<T> set_b(b.begin(), b.end());

    const auto& smaller = set_a.size() <= set_b.size() ? set_a : set_b;
    const auto& larger = set_a.size() <= set_b.size() ? set_b : set_a;

    size_t intersection = 0;
    for (const T& elem : smaller) {
        intersection += larger.contains(elem);
    }

    const auto union_size =
        static_cast<double>(set_a.size() + set_b.size() - intersection);
    return union_size ? static_cast<double>(intersection) / union_size : 0.0;
}

int main() {
    std::vector<int> vec1 = {1, 2, 3, 4, 5};
    std::vector<int> vec2 = {3, 4, 5, 6, 7};
    double similarity = jaccard_similarity<int>(vec1, vec2); // ≈ 0.4286 (3/7)

    println("Jaccard Similarity: {}", similarity);

    return 0;
}

// view ./build/optimizations.txt
