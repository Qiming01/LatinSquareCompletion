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
    std::cerr << "用法: " << program_name << " <时间限制(秒)> <随机种子> [线程数] <输入文件 >输出文件" << std::endl;
    std::cerr << "示例: " << program_name << " 600 123456 <../data/LSC.n50f750.00.txt >sln.LSC.n50f750.00.txt" << std::endl;
    std::cerr << "      " << program_name << " 600 123456 4 <../data/LSC.n50f750.00.txt >sln.LSC.n50f750.00.txt" << std::endl;
    std::cerr << "线程数为可选参数，默认使用所有可用的CPU核心" << std::endl;
}

// 验证解的冲突数
int verify_solution_conflicts(const Solution &solution) {
    const auto &grid    = solution.solution;
    const auto N        = grid.size();
    int total_conflicts = 0;

    // 计算行冲突
    int row_conflicts = 0;
    auto count        = std::make_unique<int[]>(N);
    for (const auto &row: grid) {
        std::fill_n(count.get(), N, 0);
        for (const auto val: row) {
            if (count[val] > 0) {
                row_conflicts += count[val];
            }
            count[val]++;
        }
    }

    // 计算列冲突
    int col_conflicts = 0;
    for (size_t col = 0; col < N; ++col) {
        std::fill_n(count.get(), N, 0);
        for (size_t row = 0; row < N; ++row) {
            const int val = grid[row][col];
            if (count[val] > 0) {
                col_conflicts += count[val];
            }
            count[val]++;
        }
    }

    total_conflicts = row_conflicts + col_conflicts;

    std::cerr << "解验证结果:" << std::endl;
    std::cerr << "  行冲突: " << row_conflicts << std::endl;
    std::cerr << "  列冲突: " << col_conflicts << std::endl;
    std::cerr << "  总冲突: " << total_conflicts << std::endl;

    return total_conflicts;
}

int main(int argc, char *argv[]) {
    // 检查命令行参数
    if (argc != 3 && argc != 4) {
        std::cerr << "错误: 参数数量不正确" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    // 解析命令行参数
    int time_limit_seconds   = 0;
    unsigned int random_seed = 0;
    size_t num_threads       = 0; // 0 表示使用默认值（所有核心）

    try {
        time_limit_seconds = std::stoi(argv[1]);
        random_seed        = static_cast<unsigned int>(std::stoul(argv[2]));
        if (argc == 4) {
            num_threads = static_cast<size_t>(std::stoul(argv[3]));
        }
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
    if (num_threads == 0) {
        std::cerr << "线程数: " << std::thread::hardware_concurrency() << " (自动检测)" << std::endl;
    } else {
        std::cerr << "线程数: " << num_threads << std::endl;
    }

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

    // 执行搜索（根据线程数选择单线程或并行搜索）
    if (num_threads <= 1) {
        local_search.search(latin_square, solution, max_iterations, time_limit_seconds);
    } else {
        local_search.parallel_search(latin_square, solution, num_threads, max_iterations, time_limit_seconds);
    }

    // 计算实际运行时间
    auto end_time                         = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cerr << "实际运行时间: " << elapsed.count() << " 秒" << std::endl;

    // 获取最终解
    const auto &best_solution = local_search.best_solution_;
    //
    // // 验证解的冲突数
    // std::cerr << "\n=== 输出前验证 ===" << std::endl;
    // int verified_conflicts = verify_solution_conflicts(best_solution);
    //
    // // 检查是否与算法报告的冲突数一致
    // if (verified_conflicts != best_solution.total_conflict) {
    //     std::cerr << "警告: 验证的冲突数 (" << verified_conflicts
    //               << ") 与算法报告的冲突数 (" << best_solution.total_conflict
    //               << ") 不一致!" << std::endl;
    // } else {
    //     std::cerr << "验证通过: 冲突数一致" << std::endl;
    // }
    //
    // if (verified_conflicts > 0) {
    //     std::cerr << "警告: 解仍有 " << verified_conflicts << " 个冲突，不是可行解" << std::endl;
    // } else {
    //     std::cerr << "成功: 找到可行解（无冲突）" << std::endl;
    // }
    // std::cerr << "==================\n"
    //           << std::endl;

    // 输出最终解到标准输出
    for (const auto &row: best_solution.solution) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << row[i];
        }
        std::cout << std::endl;
    }

    return 0;
}