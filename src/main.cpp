//
// Created by qiming on 25-7-17.
//
#include "latin_square/Instance.h"
#include "latin_square/ColorDomain.h"
#include <iostream>
#include <chrono>

using namespace qm::latin_square;

int main() {
    Instance instance;
    std::cin >> instance;
    ColorDomain color_domain(instance.size());
    for (const auto &assignment: instance.get_fixed()) {
        color_domain.set_fixed(assignment.row, assignment.col, assignment.num);
    }
    auto start_time = std::chrono::system_clock::now();
    color_domain.simplify();
    auto end_time = std::chrono::system_clock::now();
    std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
              << std::endl;
    std::cout << color_domain.fixed_num() << std::endl;
    return 0;
}