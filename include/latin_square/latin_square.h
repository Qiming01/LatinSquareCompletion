//
// Created by qiming on 25-7-17.
//

#pragma once

#include "color_domain.h"
#include "latin_square/instance.h"
#include "latin_square/move.h"
#include <memory>
#include <utility>

namespace qm::latin_square {
struct Solution {
    int row_conflict{};
    int column_conflict{};
    int total_conflict{}; // 一级评估
    int domain_conflict{};// 二级评估
    std::vector<std::vector<int>> solution;

    Solution()                            = default;
    Solution(const Solution &)            = default;
    Solution(Solution &&)                 = default;
    Solution &operator=(const Solution &) = default;
    Solution &operator=(Solution &&)      = default;

    explicit Solution(std::vector<std::vector<int>> solution) : solution(std::move(solution)) { calculate_conflict(); }

    [[nodiscard]] int get_color(const int row, const int col) const { return solution[row][col]; }

    void make_move(const Move &move) {
        std::swap(solution[move.row_id][move.col1], solution[move.row_id][move.col2]);
        calculate_conflict();
    }

    bool operator==(const Solution &other) const {
#ifdef COMPARE_DOMAIN_CONFLICTS
        return row_conflict == other.row_conflict &&
               column_conflict == other.column_conflict &&
               total_conflict == other.total_conflict &&
               solution == other.solution &&
               domain_conflict == other.domain_conflict;
#else
        return total_conflict == other.total_conflict;
#endif
    }


    bool operator<(const Solution &other) const {
#ifdef COMPARE_DOMAIN_CONFLICTS
        // 先比较一级评估指标 (total_conflict)
        if (total_conflict != other.total_conflict) { return total_conflict < other.total_conflict; }
        // 当一级评估指标相同时，比较二级评估指标 (domain_conflict)
        return domain_conflict < other.domain_conflict;
#else
        return total_conflict < other.total_conflict;
#endif
    }

    bool operator>(const Solution &other) const {
#ifdef COMPARE_DOMAIN_CONFLICTS
        // 先比较一级评估指标 (total_conflict)
        if (total_conflict != other.total_conflict) { return total_conflict > other.total_conflict; }
        // 当一级评估指标相同时，比较二级评估指标 (domain_conflict)
        return domain_conflict > other.domain_conflict;
#else
        return total_conflict > other.total_conflict;
#endif
    }

    bool operator<=(const Solution &other) const { return !(*this > other); }

    bool operator>=(const Solution &other) const { return !(*this < other); }
    bool operator!=(const Solution &other) const { return !(*this == other); }

    int operator-(const Solution &other) const {
        return total_conflict - other.total_conflict;
    }

    void calculate_conflict() {
        row_conflict    = 0;
        column_conflict = 0;
        total_conflict  = 0;
        if (solution.empty()) { throw std::invalid_argument("Solution is empty"); }
        const auto N = solution.size();
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
    }
};

class LatinSquare {
public:
    LatinSquare() = default;

    explicit LatinSquare(std::shared_ptr<Instance> instance) : instance_(std::move(instance)), color_domain_(instance_->size()) {
        for (const auto &assignment: instance_->get_fixed()) { color_domain_.set_fixed(assignment.row, assignment.col, assignment.num); }
        color_domain_.simplify();
    }

    Solution generate_init_solution() { return Solution{color_domain_.get_initial_solution()}; }

    // 第 i 行 第 j 列 color 颜色是否在其颜色域内
    [[nodiscard]] bool color_in_domain(const int i, const int j, const int color) const { return color_domain_.is_valid(i, j, color); }

    // 第 i 行 第 j 列是否被固定
    [[nodiscard]] bool is_fixed(const int i, const int j) const { return color_domain_(i, j).size == 1; }

    [[nodiscard]] int get_instance_size() const { return instance_->size(); }

    // private:
    std::shared_ptr<Instance> instance_;
    ColorDomain color_domain_;
};
}// namespace qm::latin_square