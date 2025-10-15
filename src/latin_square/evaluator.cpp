//
// Created by 祁明 on 2025/10/14.
//
#include "latin_square/evaluator.h"


namespace qm::latin_square {
ColColorNumTable::ColColorNumTable(const Solution &solution) { set_table(solution); }

void ColColorNumTable::set_table(const Solution &solution) {
    const auto N = solution.solution.size();
    table_.assign(N, std::vector<int>(N, 0));
    for (auto j = 0; j < N; ++j) {
        // 遍历第j列的所有元素
        for (auto i = 0; i < N; ++i) {
            const auto color = solution.solution[i][j];
            ++table_[color][j];
        }
    }
}

int ColColorNumTable::get_move_delta(const Solution &solution, const Move &move) const {
    const auto color1 = solution.get_color(move.row_id, move.col1);
    const auto color2 = solution.get_color(move.row_id, move.col2);
    const auto res    = -table_[color1][move.col1] - table_[color2][move.col2] + 2 + table_[color2][move.col1] + table_[color1][move.col2];
    return res;
}

void ColColorNumTable::make_move(const Solution &old_solution, const Move &move) {
    const auto color1 = old_solution.get_color(move.row_id, move.col1);
    const auto color2 = old_solution.get_color(move.row_id, move.col2);
    --table_[color1][move.col1];
    --table_[color2][move.col2];
    ++table_[color2][move.col1];
    ++table_[color1][move.col2];
}

ColorInDomainTable::ColorInDomainTable(const Solution &solution, const LatinSquare &latin_square) : latin_square_(latin_square) { set_table(solution, latin_square); }

void ColorInDomainTable::set_table(const Solution &solution, const LatinSquare &latin_square) {
    const auto N = solution.solution.size();
    table_.resize(N, std::vector<int>(N, 0));
    for (auto i = 0; i < N; ++i) {
        for (auto j = 0; j < N; ++j) {
            if (latin_square.color_in_domain(i, j, solution.get_color(i, j))) {
                table_[i][j] = 0;
            } else {
                table_[i][j] = 1;
            }
        }
    }
}

int ColorInDomainTable::get_move_delta(const Solution &solution, const Move &move) const {
    const auto color1 = solution.get_color(move.row_id, move.col1);
    const auto color2 = solution.get_color(move.row_id, move.col2);
    auto new_1        = latin_square_.color_in_domain(move.row_id, move.col1, color2) ? 0 : 1;
    auto new_2        = latin_square_.color_in_domain(move.row_id, move.col2, color1) ? 0 : 1;
    return new_1 + new_2 - table_[move.row_id][move.col1] - table_[move.row_id][move.col2];
}

void ColorInDomainTable::make_move(const Solution &old_solution, const Move &move) {
    const auto color1              = old_solution.get_color(move.row_id, move.col1);
    const auto color2              = old_solution.get_color(move.row_id, move.col2);
    table_[move.row_id][move.col1] = latin_square_.color_in_domain(move.row_id, move.col1, color2) ? 0 : 1;
    table_[move.row_id][move.col2] = latin_square_.color_in_domain(move.row_id, move.col2, color1) ? 0 : 1;
}

}// namespace qm::latin_square