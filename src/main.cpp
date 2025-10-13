//
// Created by qiming on 25-7-17.
//
#include "latin_square/instance.h"
#include "latin_square/color_domain.h"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace qm::latin_square;

bool check_instance(const Instance &instance) {
    try {
        ColorDomain color_domain(instance.size());
        // 先设置所有固定值
        for (const auto &assignment: instance.get_fixed()) { color_domain.set_fixed(assignment.row, assignment.col, assignment.num); }
        // 然后获取初始解
        auto solution = color_domain.get_initial_solution();
        return true;// 成功获取解
    } catch (const std::exception &e) {
        return false;// 获取解失败
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
                bool success    = check_instance(instance);
                auto end_time   = std::chrono::system_clock::now();

                if (success) {
                    std::cout << entry.path() << ": OK, took "
                            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                            << "ms"
                            << std::endl;
                } else { std::cout << entry.path() << ": FAILED" << std::endl; }
            } catch (const std::exception &e) { std::cerr << entry.path() << ": Exception - " << e.what() << std::endl; }
        }
    }
}

void test_simplify(const std::string &dir_path, const std::string &csv_path) {
    std::ofstream csv_out(csv_path);
    if (!csv_out.is_open()) {
        std::cerr << "无法打开输出文件: " << csv_path << std::endl;
        return;
    }

    // CSV header
    csv_out << "filename,fixed_cells,total_domain_size,time_ms\n";

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

                ColorDomain color_domain(instance.size());
                for (const auto &assignment: instance.get_fixed()) { color_domain.set_fixed(assignment.row, assignment.col, assignment.num); }

                color_domain.simplify();
                int fixed_num         = color_domain.fixed_num();
                int total_domain_size = color_domain.total_domain_size();

                auto end_time     = std::chrono::system_clock::now();
                long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time).count();

                // 写入一行 CSV 结果
                csv_out << entry.path().filename().string() << ","
                        << fixed_num << ","
                        << total_domain_size << ","
                        << elapsed << "\n";

                std::cout << entry.path().filename().string() << ": OK, took "
                        << elapsed << "ms" << std::endl;
            } catch (const std::exception &e) { std::cerr << entry.path() << ": Exception - " << e.what() << std::endl; }
        }
    }

    csv_out.close();
    std::cout << "结果已保存至: " << csv_path << std::endl;
}

void test_instance() {
    auto start_time = std::chrono::system_clock::now();
    Instance instance;
    std::cin >> instance;
    ColorDomain color_domain(instance.size());
    for (const auto &assignment: instance.get_fixed()) { color_domain.set_fixed(assignment.row, assignment.col, assignment.num); }
    color_domain.simplify();
    auto res = color_domain.get_initial_solution();
    for (const auto &solution: res) {
        for (const auto &num: solution) { std::cout << num << " "; }
        std::cout << std::endl;
    }
    auto end_time = std::chrono::system_clock::now();
    std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
            << std::endl;
    std::cout << color_domain.fixed_num() << std::endl;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    test_instance();
    // test_simplify(R"(C:\Users\qiming\Documents\GitHub\LatinSquareCompletion\Instance)", "output.csv");
    //check_all_instance(R"(C:\Users\qiming\Documents\GitHub\LatinSquareCompletion\Instance)");

    return 0;
}