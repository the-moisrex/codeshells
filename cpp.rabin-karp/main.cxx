import standard;

#include <concepts>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <valarray>
#include <vector>

// AI generated code:
// rabin_karp.cpp
// Compile: g++ -std=c++26 -O2 rabin_karp.cpp -o rabin_karp
using namespace std;

// Return all starting indices where `pattern` occurs in `text`.
vector<size_t> rabin_karp_find_all(string_view text, string_view pattern) {
    vector<size_t> result;
    size_t n = text.size();
    size_t m = pattern.size();

    // Edge cases
    if (m == 0) { // empty pattern -> match at every position [0..n]
        result.reserve(n + 1);
        for (size_t i = 0; i <= n; ++i)
            result.push_back(i);
        return result;
    }
    if (m > n)
        return result;

    // Parameters for polynomial rolling hash
    constexpr uint64_t base = 257; // base > max char value (simple choice)
    constexpr uint64_t mod = 1'000'000'007ULL; // large prime modulus

    // Compute base^(m-1) % mod for removing leading char contribution
    uint64_t base_m1 = 1;
    for (size_t i = 0; i + 1 < m; ++i) {
        base_m1 = (base_m1 * base) % mod;
    }

    // Compute hash for pattern and first window of text
    auto hash_of = [&](string_view s, size_t len) {
        uint64_t h = 0;
        for (size_t i = 0; i < len; ++i) {
            h = (h * base + static_cast<unsigned char>(s[i])) % mod;
        }
        return h;
    };

    uint64_t pat_hash = hash_of(pattern, m);
    uint64_t window_hash = hash_of(text, m);

    // Check the first window explicitly
    if (window_hash == pat_hash) {
        if (text.substr(0, m) == pattern)
            result.push_back(0);
    }

    // Slide window: update hash in O(1) time per step
    for (size_t i = 1; i + m <= n; ++i) {
        // remove leading char and add trailing char
        uint64_t leading = static_cast<unsigned char>(text[i - 1]);
        uint64_t trailing = static_cast<unsigned char>(text[i + m - 1]);

        // subtract contribution of leading char:
        // window_hash = (window_hash - leading * base^{m-1}) * base + trailing
        // (mod mod) do subtraction carefully to avoid negative underflow (work
        // in uint64_t with mod)
        uint64_t tmp = (window_hash + mod - (leading * base_m1) % mod) % mod;
        tmp = (tmp * base + trailing) % mod;
        window_hash = tmp;

        if (window_hash == pat_hash) {
            // verify to avoid false positive from hash collision
            if (text.substr(i, m) == pattern)
                result.push_back(i);
        }
    }

    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Example usage:
    string text = "ababcabcababc";
    string pattern = "abc";

    auto matches = rabin_karp_find_all(text, pattern);
    cout << "Text:    " << text << "\n";
    cout << "Pattern: " << pattern << "\n";
    cout << "Matches at indices:";
    for (size_t idx : matches)
        cout << ' ' << idx;
    cout << '\n';

    // Interactive: read text and pattern from stdin if available
    // Uncomment below to accept input from user:
    /*
    string t, p;
    if (getline(cin, t) && getline(cin, p)) {
        auto m2 = rabin_karp_find_all(t, p);
        cout << "Found " << m2.size() << " occurrences at: ";
        for (auto i : m2) cout << i << " ";
        cout << "\n";
    }
    */
    return 0;
}

// view ./build/optimizations.txt
