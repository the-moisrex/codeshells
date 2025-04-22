import standard;

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <print>
#include <span>
#include <stdexcept>
#include <vector>

using namespace std;

// https://en.wikipedia.org/wiki/Euclidean_distance
float euclidean_distance(std::span<float> embedding1,
                         std::span<float> embedding2) {
    if (embedding1.size() != embedding2.size()) {
        throw std::invalid_argument("Embedding sizes should be equal.");
    }

    float sum = 0;

    std::size_t embeddingSize = embedding1.size();
    for (std::size_t i = 0; i < embeddingSize; i++) {
        float distance = embedding2[i] - embedding1[i];
        sum += distance * distance;
    }

    float euclideanDistance = std::sqrt(sum);

    return euclideanDistance;
}

int main() {
    vector<float> e1{0.1, 0.2, 0.3};
    vector<float> e2{0.0, 0.0, 0.0};

    println("Euclidean Distance: {}", euclidean_distance(e1, e2));

    return 0;
}

// view ./build/optimizations.txt
