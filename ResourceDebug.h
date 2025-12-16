#ifndef RESOURCE_DEBUG_H
#define RESOURCE_DEBUG_H

#include <iostream>
#include <cstdint>

#include <string>
#include <unordered_set>

const std::string RES_Names[] = {
    "EMPTY",
    "IF_stage",
    "ID_stage",
    "EX_stage",
    "WB_stage",
    "IF1_substage",
    "IF2_substage",
    "F_SIZE"};

/* void print_set(std::unordered_set<uint64_t>& input) {
    std::cout << "{";

    if (input.empty()) {
        std::cout << "}\t";
        return;
    }

    for (auto it = input.begin(); it != input.end();) {
        std::cout << *it;
        if (++it != input.end()) {
            std::cout << ", ";
        }
    }

    std::cout << "}\t";
}; */

#endif