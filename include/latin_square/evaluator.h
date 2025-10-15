//
// Created by 祁明 on 2025/10/14.
//

#ifndef LATINSQUARECOMPLETION_EVALUATOR_H
#define LATINSQUARECOMPLETION_EVALUATOR_H
#include "latin_square/latin_square.h"
#include "vec_set.h"

namespace qm::latin_square {


/**
 * @brief 列内颜色数记录表
 * @details 使用 VecSet 存储每个 (颜色, 列) 对应的行集合
 * table_[color][col] 存储在第 col 列使用 color 颜色的所有行
 * size 表示该 (颜色, 列) 对的行数，用于快速判断冲突
 */
struct ColColorNumTable {
    ColColorNumTable() = default;

    explicit ColColorNumTable(const Solution &solution);

    void set_table(const Solution &solution);

    [[nodiscard]] int get_move_delta(const Solution &solution, const Move &move) const;

    // warn: 先更新记录表，再更新解
    // 返回值：受影响的 (颜色, 列) 对，用于增量更新冲突节点
    struct AffectedCell {
        int color;
        int col;
    };
    std::vector<AffectedCell> make_move(const Solution &old_solution, const Move &move);

    [[nodiscard]] bool is_conflict_grid(int color, int col) const {
        return table_[color][col].size() > 1;
    }

    // 获取在第 col 列使用 color 颜色的所有行
    [[nodiscard]] const VecSet &get_rows(int color, int col) const {
        return table_[color][col];
    }

    std::vector<std::vector<VecSet>> table_;// table_[color][col] = 使用该颜色的行集合
};

/**
 * @brief 每个格子的当前颜色是否在颜色域内，0 表示在，1 表示不在
 */
struct ColorInDomainTable {
    ColorInDomainTable() = default;
    explicit ColorInDomainTable(const Solution &solution, const LatinSquare &latin_square);
    void set_table(const Solution &solution, const LatinSquare &latin_square);

    [[nodiscard]] int get_move_delta(const Solution &solution, const Move &move) const;

    // warn: 先更新记录表，再更新解
    void make_move(const Solution &old_solution, const Move &move);

    [[nodiscard]] bool is_in_domain(int i, int j) const { return table_[i][j] == 0; }

    LatinSquare latin_square_;
    std::vector<std::vector<int>> table_;
};

class Evaluator {
    friend class LocalSearch;

public:
    Evaluator() = default;
    // 一级评估函数
    explicit Evaluator(const LatinSquare &latin_square, const Solution &solution) : col_color_num_table_(solution), color_in_domain_table_(solution, latin_square) {}
    [[nodiscard]] int evaluate_conflict_delta(const Solution &solution, const Move &move) const { return col_color_num_table_.get_move_delta(solution, move); }
    // 二级评估函数
    [[nodiscard]] int evaluate_domain_delta(const Solution &solution, const Move &move) const { return color_in_domain_table_.get_move_delta(solution, move); }
    // 对评估器进行更新，需要在对 solution 进行邻域动作之前
    void update(const Solution &old_solution, const Move &move) {
        col_color_num_table_.make_move(old_solution, move);
        color_in_domain_table_.make_move(old_solution, move);
    }

    [[nodiscard]] bool is_conflict_grid(int color, int j) const { return col_color_num_table_.table_[color][j].size() > 1; }

private:
    ColColorNumTable col_color_num_table_;
    ColorInDomainTable color_in_domain_table_;
};

}// namespace qm::latin_square
#endif// LATINSQUARECOMPLETION_EVALUATOR_H
