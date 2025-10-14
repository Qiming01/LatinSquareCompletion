//
// Created by qiming on 25-7-17.
//
#include "latin_square/color_domain.h"
#include "latin_square/instance.h"
#include "latin_square/latin_square.h"
#include "latin_square/local_search.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace qm::latin_square;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    const auto instance = std::make_shared<Instance>();
    std::cin >> *instance;
    auto latin_square = LatinSquare(instance);
    auto solution     = latin_square.generate_init_solution();
    LocalSearch local_search;
    local_search.search(latin_square, solution, 10000);
    return 0;
}