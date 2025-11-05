
#include <iostream>
#include <cstdint>
#include <vector>
#include <utility>

void print_vec(std::vector<uint64_t>& input) {
    std::cout << "{";

    if (input.empty()) {
        std::cout << "}\t";
        return;
    }

    for (size_t i = 0; i < input.size() - 1; i++) {
        std::cout << input[i] << ",";
    }
    std::cout << input[input.size() - 1] << "}\t";
}

// Custom comparator for the priority queue.
struct ComparePairs {
    bool operator()(const std::pair<uint64_t, uint64_t>& a, const std::pair<uint64_t, uint64_t>& b) {
        // Prefer the first element in descending order.
        if (a.first != b.first) {
            return a.first < b.first; // greater first
        }
        // Prefer the second element in ascending order.
        return a.second > b.second; // lower second
    }
};