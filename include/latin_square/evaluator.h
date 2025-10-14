//
// Created by 祁明 on 2025/10/14.
//

#ifndef LATINSQUARECOMPLETION_EVALUATOR_H
#define LATINSQUARECOMPLETION_EVALUATOR_H
#include "latin_square/latin_square.h"
namespace qm::latin_square {


/**
 * @brief 列内颜色数记录表
 * @size 大小：N（颜色数） * N（列数）
 */
class RowColorNumTable {
public:
    RowColorNumTable() = default;

    explicit RowColorNumTable(const Solution &solution);

    void set_table(const Solution &solution);

    [[nodiscard]] int get_move_delta(const Solution &solution, const Move &move) const;

    // warn: 先更新记录表，再更新解
    void make_move(const Solution &old_solution, const Move &move);

private:
    std::vector<std::vector<int>> table_;
};

/**
 * @brief 每个格子的当前颜色是否在颜色域内，1 表示在，0 表示不在
 */
class ColorInDomainTable {
public:
    explicit ColorInDomainTable(const Solution &solution, const LatinSquare &latin_square);
    void set_table(const Solution &solution, const LatinSquare &latin_square);

    [[nodiscard]] int get_move_delta(const Solution &solution, const Move &move) const;

    // warn: 先更新记录表，再更新解
    void make_move(const Solution &old_solution, const Move &move);

private:
    LatinSquare latin_square_;
    std::vector<std::vector<int>> table_;
};

class Evaluator {
public:
    // 一级评估函数
    explicit Evaluator(const LatinSquare &latin_square, const Solution &solution) : row_color_num_table_(solution), color_in_domain_table_(solution, latin_square) {}
    [[nodiscard]] int evaluate_conflict_delta(const Solution &solution, const Move &move) const {
        return row_color_num_table_.get_move_delta(solution, move);
    }
    // 二级评估函数
    [[nodiscard]] int evaluate_domain_delta(const Solution &solution, const Move &move) const {
        return color_in_domain_table_.get_move_delta(solution, move);
    }
    // 对评估器进行更新，需要在对 solution 进行邻域动作之前
    void update(const Solution &old_solution, const Move &move) {
        row_color_num_table_.make_move(old_solution, move);
        color_in_domain_table_.make_move(old_solution, move);
    }

private:
    RowColorNumTable row_color_num_table_;
    ColorInDomainTable color_in_domain_table_;
};

}// namespace qm::latin_square
#endif// LATINSQUARECOMPLETION_EVALUATOR_H
