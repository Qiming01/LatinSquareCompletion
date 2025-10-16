//
// Created by 祁明 on 2025/10/14.
//

#ifndef LATINSQUARECOMPLETION_LOCAL_SEARCH_H
#define LATINSQUARECOMPLETION_LOCAL_SEARCH_H

#include "latin_square/evaluator.h"
#include "latin_square/latin_square.h"
#include "latin_square/move.h"
#include "latin_square/vec_set.h"
#include "utils/ThreadPool.h"

namespace qm::latin_square {

class TabuList {
public:
    TabuList() = default;

    explicit TabuList(int N) : N_(N) {
        // 初始化一维数组，大小为 N * N * N
        tabu_list_.resize(N * N * N, 0);
    }

    [[nodiscard]] bool is_tabu(int i, int j, int color, unsigned long long current_iteration) const {
        // 检查索引是否在有效范围内
        assert(i >= 0 && i < N_ && j >= 0 && j < N_ && color >= 0 && color < N_);

        // 计算一维索引：i * (N*N) + j * N + color
        const int index = i * N_ * N_ + j * N_ + color;
        return current_iteration < tabu_list_[index];
    }

    void make_tabu(int i, int j, int color, unsigned long long target_iteration) {
        // 检查索引是否在有效范围内
        assert(i >= 0 && i < N_ && j >= 0 && j < N_ && color >= 0 && color < N_);

        // 计算一维索引：i * (N*N) + j * N + color
        int index         = i * N_ * N_ + j * N_ + color;
        tabu_list_[index] = target_iteration;
    }

    void clear_tabu() {
        // 重置所有禁忌状态为0（非禁忌）
        std::ranges::fill(tabu_list_, 0);
    }

    // 获取底层数组大小的方法
    [[nodiscard]] int size() const { return N_; }

    // 获取内存使用情况（字节）
    [[nodiscard]] size_t memory_usage() const { return tabu_list_.size() * sizeof(unsigned long long); }

private:
    int N_{};// 问题规模
    // 一维数组表示三维结构：tabu_list_[i * N*N + j * N + color] = target_iteration
    std::vector<unsigned long long> tabu_list_;
};

class LocalSearch {
public:
    LocalSearch() = default;
    
    // 单线程搜索
    void search(const LatinSquare &latin_square, const Solution &solution, unsigned long long max_iteration = 0, int time_limit_seconds = 0);
    
    // 多线程并行搜索（多次重启）
    void parallel_search(const LatinSquare &latin_square, const Solution &solution, 
                        size_t num_threads, unsigned long long max_iteration = 0, int time_limit_seconds = 0);
    
    Solution best_solution_;  // 公开最优解，供外部访问

private:
    unsigned long long iteration_{};
    Solution current_solution_;
    TabuList tabu_list_;
    Evaluator evaluator_;
    std::vector<VecSet> row_conflict_grid_;
    std::vector<VecSet> row_nonconflict_grid_;
    VecSet conflict_nodes_;  // 所有冲突节点的集合（按节点ID存储）
    int rt{};
    int accu{};
    
    // 预分配数组用于存储候选移动
    std::vector<Move> equal_nontabu_moves_;
    std::vector<Move> equal_tabu_moves_;
    
    Move find_move();
    void make_move(const Move &move);
    void set_row_conflict_grid_(const Solution &solution);
    void update_row_conflict_grid_incremental_(const std::vector<ColColorNumTable::AffectedCell> &affected_cells);
    [[nodiscard]] bool is_tabu(const Move &move, int conflict_num) const;
    void set_tabu(const Move &move);
    
    // for debug
    void verify_conflict_grid() const;

    // for debug
    void check_solution_conflict_number() const {
        // 精确计算冲突边数量
        int row_conflict     = 0;
        int column_conflict  = 0;
        int total_conflict   = 0;
        const auto &solution = current_solution_.solution;
        const auto N         = solution.size();
        // 计算行冲突
        auto existed = std::make_unique<int[]>(N);
        for (const auto &row: solution) {
            std::fill_n(existed.get(), N, 0);
            for (const auto val: row) {
                if (existed[val] > 0) { row_conflict += existed[val]; }
                existed[val]++;
            }
        }
        // 计算列冲突
        for (size_t j = 0; j < N; ++j) {
            // 遍历列
            std::fill_n(existed.get(), N, false);
            for (size_t i = 0; i < N; ++i) {
                // 遍历行
                const int val = solution[i][j];
                if (existed[val] > 0) { column_conflict += existed[val]; }
                existed[val]++;
            }
        }
        total_conflict = row_conflict + column_conflict;
        if (total_conflict != current_solution_.total_conflict) {
            throw std::runtime_error("冲突边个数计算错误");
        }
        auto &domain        = evaluator_.color_in_domain_table_.latin_square_.color_domain_;
        int domain_conflict = 0;
        for (auto i = 0; i < N; ++i) {
            for (auto j = 0; j < N; ++j) {
                auto val = solution[i][j];
                if (domain(i, j).bits[val] == false) {
                    domain_conflict++;
                }
            }
        }
        if (domain_conflict != current_solution_.domain_conflict) {
            throw std::runtime_error("domain冲突个数计算错误");
        }
    }
};
}// namespace qm::latin_square

#endif// LATINSQUARECOMPLETION_LOCAL_SEARCH_H
