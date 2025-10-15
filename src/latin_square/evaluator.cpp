//
// Created by 祁明 on 2025/10/14.
//
#include "latin_square/evaluator.h"


namespace qm::latin_square {
ColColorNumTable::ColColorNumTable(const Solution &solution) { set_table(solution); }

void ColColorNumTable::set_table(const Solution &solution) {
    const auto N = solution.solution.size();
    table_.clear();
    table_.resize(N);
    for (auto color = 0; color < N; ++color) {
        table_[color].resize(N, VecSet(N));  // 每个 (颜色, 列) 对应一个 VecSet，存储行号
    }
    
    // 遍历所有格子，将行号添加到对应的 (颜色, 列) 集合中
    for (auto row = 0; row < N; ++row) {
        for (auto col = 0; col < N; ++col) {
            const auto color = solution.solution[row][col];
            table_[color][col].insert(row);
        }
    }
}

int ColColorNumTable::get_move_delta(const Solution &solution, const Move &move) const {
    const auto color1 = solution.get_color(move.row_id, move.col1);
    const auto color2 = solution.get_color(move.row_id, move.col2);
    
    // 使用 size() 获取颜色数量
    const auto count_c1_col1 = table_[color1][move.col1].size();
    const auto count_c2_col2 = table_[color2][move.col2].size();
    const auto count_c2_col1 = table_[color2][move.col1].size();
    const auto count_c1_col2 = table_[color1][move.col2].size();
    
    const auto res = -count_c1_col1 - count_c2_col2 + 2 + count_c2_col1 + count_c1_col2;
    return res;
}

std::vector<ColColorNumTable::AffectedCell> ColColorNumTable::make_move(const Solution &old_solution, const Move &move) {
    const auto color1 = old_solution.get_color(move.row_id, move.col1);
    const auto color2 = old_solution.get_color(move.row_id, move.col2);
    
    // 更新表：移除旧的行-颜色关系，添加新的行-颜色关系
    table_[color1][move.col1].erase(move.row_id);
    table_[color2][move.col2].erase(move.row_id);
    table_[color2][move.col1].insert(move.row_id);
    table_[color1][move.col2].insert(move.row_id);
    
    // 返回受影响的 (颜色, 列) 对
    std::vector<AffectedCell> affected;
    affected.reserve(4);
    affected.push_back({color1, move.col1});
    affected.push_back({color2, move.col2});
    affected.push_back({color2, move.col1});
    affected.push_back({color1, move.col2});
    
    return affected;
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