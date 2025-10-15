//
// Created by 祁明 on 2025/10/14.
//
#include "latin_square/local_search.h"

#include "utils/RandomGenerator.h"

#include <limits>

namespace qm::latin_square {
void LocalSearch::search(const LatinSquare &latin_square, const Solution &solution, const unsigned long long max_iteration) {
    current_solution_ = solution;
    best_solution_    = solution;
    iteration_        = 0;
    tabu_list_        = TabuList{latin_square.get_instance_size()};
    evaluator_        = Evaluator{latin_square, solution};
    accu              = 0;
    rt                = 10;
    while (iteration_ < max_iteration) {
        set_row_conflict_grid_(current_solution_);

        auto move = find_move();
        make_move(move);
        if (current_solution_.total_conflict == 0) {
            std::clog << "Iteration: " << iteration_ << " conflict = 0, return." << std::endl;
            return;
        }
        if (current_solution_ <= best_solution_) { best_solution_ = current_solution_; }
        if (iteration_ % 10000 == 0) { std::clog << "Iteration: " << iteration_ << " conflict = " << current_solution_.total_conflict << std::endl; }

        if (current_solution_ - best_solution_ > rt) {
            std::cerr << "重启" << std::endl;
            tabu_list_ = TabuList{latin_square.get_instance_size()};
            // 使用历史最优解替换当前解
            current_solution_ = best_solution_;
            evaluator_        = Evaluator{latin_square, current_solution_};
            // 扰动 todo
            // 如果重启阈值没有到达上限
            static constexpr int rtub  = 15;
            static constexpr int accub = 1000;
            if (rt < rtub) {
                ++accu;// 累计重启次数加一
                if (accu == accub) {
                    ++rt;// 增大重启阈值
                    accu = 0;
                }
            }
        }
        ++iteration_;
    }
}

Move LocalSearch::find_move() {
    // 遍历每一行
    const int N = static_cast<int>(current_solution_.solution.size());
    Move best_non_tabu_move{-1, -1, -1};
    Move best_tabu_move{-1, -1, -1};
    int best_non_tabu_move_delta1 = std::numeric_limits<int>::max();
    int best_non_tabu_move_delta2 = std::numeric_limits<int>::max();
    int best_tabu_move_delta1     = std::numeric_limits<int>::max();
    int best_tabu_move_delta2     = std::numeric_limits<int>::max();
    int non_tabu_move_num         = 0;
    int tabu_move_num             = 0;

    for (auto row = 0; row < N; ++row) {
        // 冲突节点 - 冲突节点
        for (auto i = 0; i < row_conflict_grid_[row].size(); ++i) {
            const auto col1 = row_conflict_grid_[row][i];
            for (auto j = i + 1; j < row_conflict_grid_[row].size(); ++j) {
                const auto col2 = row_conflict_grid_[row][j];
                Move move{row, col1, col2};

                auto move_delta1 = evaluator_.evaluate_conflict_delta(current_solution_, move);
                auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);

                if (is_tabu(move, current_solution_.total_conflict + move_delta1)) {
                    // 禁忌移动
                    if (move_delta1 < best_tabu_move_delta1 ||
                        (move_delta1 == best_tabu_move_delta1 && move_delta2 < best_tabu_move_delta2)) {
                        best_tabu_move_delta1 = move_delta1;
                        best_tabu_move_delta2 = move_delta2;
                        best_tabu_move        = move;
                        tabu_move_num         = 1;
                    } else if (move_delta1 == best_tabu_move_delta1 && move_delta2 == best_tabu_move_delta2) {
                        tabu_move_num++;
                        if (randomInt(tabu_move_num) == 0) { best_tabu_move = move; }
                    }
                } else {
                    // 非禁忌移动
                    if (move_delta1 < best_non_tabu_move_delta1 ||
                        (move_delta1 == best_non_tabu_move_delta1 && move_delta2 < best_non_tabu_move_delta2)) {
                        best_non_tabu_move_delta1 = move_delta1;
                        best_non_tabu_move_delta2 = move_delta2;
                        best_non_tabu_move        = move;
                        non_tabu_move_num         = 1;
                    } else if (move_delta1 == best_non_tabu_move_delta1 && move_delta2 == best_non_tabu_move_delta2) {
                        non_tabu_move_num++;
                        if (randomInt(non_tabu_move_num) == 0) { best_non_tabu_move = move; }
                    }
                }
            }
        }

        // 冲突节点 - 非冲突节点
        for (const auto col1: row_conflict_grid_[row]) {
            for (const auto col2: row_nonconflict_grid_[row]) {
                Move move{row, col1, col2};

                auto move_delta1 = evaluator_.evaluate_conflict_delta(current_solution_, move);
                auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);

                if (is_tabu(move, current_solution_.total_conflict + move_delta1)) {
                    // 禁忌移动
                    if (move_delta1 < best_tabu_move_delta1 ||
                        (move_delta1 == best_tabu_move_delta1 && move_delta2 < best_tabu_move_delta2)) {
                        best_tabu_move_delta1 = move_delta1;
                        best_tabu_move_delta2 = move_delta2;
                        best_tabu_move        = move;
                        tabu_move_num         = 1;
                    } else if (move_delta1 == best_tabu_move_delta1 && move_delta2 == best_tabu_move_delta2) {
                        tabu_move_num++;
                        if (randomInt(tabu_move_num) == 0) { best_tabu_move = move; }
                    }
                } else {
                    // 非禁忌移动
                    if (move_delta1 < best_non_tabu_move_delta1 ||
                        (move_delta1 == best_non_tabu_move_delta1 && move_delta2 < best_non_tabu_move_delta2)) {
                        best_non_tabu_move_delta1 = move_delta1;
                        best_non_tabu_move_delta2 = move_delta2;
                        best_non_tabu_move        = move;
                        non_tabu_move_num         = 1;
                    } else if (move_delta1 == best_non_tabu_move_delta1 && move_delta2 == best_non_tabu_move_delta2) {
                        non_tabu_move_num++;
                        if (randomInt(non_tabu_move_num) == 0) { best_non_tabu_move = move; }
                    }
                }
            }
        }
    }

    // 选择最佳移动：特赦规则 - 如果禁忌移动比历史最优解更好，则选择禁忌移动
    if (best_tabu_move_delta1 < best_solution_.total_conflict - current_solution_.total_conflict &&
        best_tabu_move_delta1 < best_non_tabu_move_delta1) {
        return best_tabu_move;
    }
    return best_non_tabu_move;
}

void LocalSearch::make_move(const Move &move) {
    auto move_delta1 = evaluator_.evaluate_conflict_delta(current_solution_, move);
    auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
    set_tabu(move);
    evaluator_.color_in_domain_table_.make_move(current_solution_, move);
    evaluator_.col_color_num_table_.make_move(current_solution_, move);
    current_solution_.total_conflict += move_delta1;
    current_solution_.domain_conflict += move_delta2;
    std::swap(current_solution_.solution[move.row_id][move.col1], current_solution_.solution[move.row_id][move.col2]);
}

void LocalSearch::set_row_conflict_grid_(const Solution &solution) {
    const int N = static_cast<int>(solution.solution.size());
    row_conflict_grid_.clear();
    row_conflict_grid_.resize(N, VecSet{N});
    row_nonconflict_grid_.clear();
    row_nonconflict_grid_.resize(N, VecSet{N});
    for (auto row = 0; row < N; ++row) {
        for (auto col = 0; col < N; ++col) {
            if (evaluator_.color_in_domain_table_.latin_square_.is_fixed(row, col)) { continue; }
            if (evaluator_.is_conflict_grid(solution.get_color(row, col), col)) {
                row_conflict_grid_[row].insert(col);
            } else {
                row_nonconflict_grid_[row].insert(col);
            }
        }
    }
}

bool LocalSearch::is_tabu(const Move &move, int conflict_num) const {
    // if (conflict_num < best_solution_.total_conflict) { return false; }
    const auto color1 = current_solution_.solution[move.row_id][move.col1];
    const auto color2 = current_solution_.solution[move.row_id][move.col2];
    // 查找交换颜色后是否在禁忌表内（回到原点）
    return tabu_list_.is_tabu(move.row_id, move.col1, color2, iteration_) || tabu_list_.is_tabu(move.row_id, move.col2, color1, iteration_);
}

void LocalSearch::set_tabu(const Move &move) {
    const auto color1 = current_solution_.solution[move.row_id][move.col1];
    const auto color2 = current_solution_.solution[move.row_id][move.col2];
    // 禁忌当前的颜色
    constexpr double alpha                     = 0.4;
    const auto target_iteration_without_random = static_cast<unsigned long long>(alpha * current_solution_.total_conflict) + iteration_;
    tabu_list_.make_tabu(move.row_id, move.col1, color1, target_iteration_without_random + randomInt(10));
    if (evaluator_.is_conflict_grid(color2, move.col2))
        tabu_list_.make_tabu(move.row_id, move.col2, color2, target_iteration_without_random + randomInt(10));
}
}// namespace qm::latin_square