//
// Created by qiming on 25-7-17.
//
#include "latin_square/color_domain.h"
#include "latin_square/instance.h"
#include "latin_square/latin_square.h"
#include "latin_square/local_search.h"
#include "utils/RandomGenerator.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace qm::latin_square;

void print_usage(const char *program_name) {
    std::cerr << "用法: " << program_name << " <时间限制(秒)> <随机种子> <输入文件 >输出文件" << std::endl;
    std::cerr << "示例: " << program_name << " 600 123456 <../data/LSC.n50f750.00.txt >sln.LSC.n50f750.00.txt" << std::endl;
}

int main(int argc, char *argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        std::cerr << "错误: 参数数量不正确" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    // 解析命令行参数
    int time_limit_seconds   = 0;
    unsigned int random_seed = 0;

    try {
        time_limit_seconds = std::stoi(argv[1]);
        random_seed        = static_cast<unsigned int>(std::stoul(argv[2]));
    } catch (const std::exception &e) {
        std::cerr << "错误: 参数解析失败 - " << e.what() << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    if (time_limit_seconds <= 0) {
        std::cerr << "错误: 时间限制必须为正数" << std::endl;
        return 1;
    }

    std::cerr << "时间限制: " << time_limit_seconds << " 秒" << std::endl;
    std::cerr << "随机种子: " << random_seed << std::endl;

    // 加速输入输出
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // 从标准输入读取实例
    const auto instance = std::make_shared<Instance>();
    std::cin >> *instance;

    qm::setRandomSeed(random_seed);


    // 初始化拉丁方和解
    auto latin_square = LatinSquare(instance);
    auto solution     = latin_square.generate_init_solution();

    if (solution.total_conflict == 0) {
        for (const auto &row: solution.solution) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << row[i];
            }
            std::cout << std::endl;
        }
        return 0;
    }

    // 设置随机种子

    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建局部搜索对象
    LocalSearch local_search;

    // 计算最大迭代次数（设置一个足够大的值，实际由时间限制控制）
    unsigned long long max_iterations = 100000000000ULL;

    // 执行搜索（传递时间限制）
    local_search.search(latin_square, solution, max_iterations, time_limit_seconds);

    // 计算实际运行时间
    auto end_time                         = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cerr << "实际运行时间: " << elapsed.count() << " 秒" << std::endl;

    // 输出最终解到标准输出
    const auto &best_solution = local_search.best_solution_;
    // std::cout << best_solution.solution.size() << std::endl;
    for (const auto &row: best_solution.solution) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << row[i];
        }
        std::cout << std::endl;
    }

    return 0;
}