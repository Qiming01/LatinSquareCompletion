//
// Created by qiming on 25-7-17.
//
#include "latin_square/Instance.h"
#include "latin_square/ColorDomain.h"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace qm::latin_square;

bool check_instance(const Instance &instance) {
    try {
        ColorDomain color_domain(instance.size());
        // 先设置所有固定值
        for (const auto &assignment: instance.get_fixed()) {
            color_domain.set_fixed(assignment.row, assignment.col, assignment.num);
        }
        // 然后获取初始解
        auto solution = color_domain.getInitialSolution();
        return true; // 成功获取解
    } catch (const std::exception &e) {
        return false; // 获取解失败
    }
}

void check_all_instance(const std::string &dir_path) {
    for (const auto &entry: std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            try {
                Instance instance;
                std::ifstream input_file(entry.path());
                if (!input_file.is_open()) {
                    std::cerr << "无法打开文件: " << entry.path() << std::endl;
                    continue;
                }

                input_file >> instance;
                input_file.close();

                auto start_time = std::chrono::system_clock::now();
                bool success = check_instance(instance);
                auto end_time = std::chrono::system_clock::now();

                if (success) {
                    std::cout << entry.path() << ": OK, took "
                              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                              << "ms"
                              << std::endl;
                } else {
                    std::cout << entry.path() << ": FAILED" << std::endl;
                }
            } catch (const std::exception &e) {
                std::cerr << entry.path() << ": Exception - " << e.what() << std::endl;
            }
        }
    }
}

void test_instance() {
    Instance instance;
    std::cin >> instance;
    ColorDomain color_domain(instance.size());
    for (const auto &assignment: instance.get_fixed()) {
        color_domain.set_fixed(assignment.row, assignment.col, assignment.num);
    }
    auto start_time = std::chrono::system_clock::now();
    color_domain.simplify();
    auto res = color_domain.getInitialSolution();
    for (const auto &solution: res) {
        for (const auto &num: solution) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }
    auto end_time = std::chrono::system_clock::now();
    std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
              << std::endl;
    std::cout << color_domain.fixed_num() << std::endl;
}

int main() {
    test_instance();
    //check_all_instance(R"(C:\Users\qiming\Documents\GitHub\LatinSquareCompletion\Instance)");

    return 0;
}