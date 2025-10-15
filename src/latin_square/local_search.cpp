//
// Created by 祁明 on 2025/10/14.
//
#include "latin_square/local_search.h"

#include "utils/RandomGenerator.h"

#include <chrono>
#include <iomanip>
#include <limits>

namespace qm::latin_square {
void LocalSearch::search(const LatinSquare &latin_square, const Solution &solution, const unsigned long long max_iteration, const int time_limit_seconds) {
    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    current_solution_ = solution;
    best_solution_    = solution;
    iteration_        = 0;
    tabu_list_        = TabuList{latin_square.get_instance_size()};
    evaluator_        = Evaluator{latin_square, solution};
    accu              = 0;
    rt                = 10;

    // 初始化冲突节点集合（只在开始时执行一次）
    set_row_conflict_grid_(current_solution_);

    while (iteration_ < max_iteration) {
        // 检查时间限制
        if (time_limit_seconds > 0) {
            auto current_time                     = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = current_time - start_time;
            if (elapsed.count() >= time_limit_seconds) {
                std::clog << "达到时间限制 " << time_limit_seconds << " 秒，搜索终止" << std::endl;
                std::clog << "最终冲突数: " << best_solution_.total_conflict << std::endl;
                return;
            }
        }
        auto move = find_move();
        make_move(move);
        if (current_solution_.total_conflict == 0) {
            // 计算求解时间
            auto end_time                         = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;
            std::clog << "Iteration: " << iteration_ << " conflict = 0, return." << std::endl;
            std::clog << "求解时间: " << std::fixed << std::setprecision(3) << elapsed.count() << " s" << std::endl;
            return;
        }
        if (current_solution_ <= best_solution_) { best_solution_ = current_solution_; }
        // if (iteration_ % 10000 == 0) { std::clog << "Iteration: " << iteration_ << " conflict = " << current_solution_.total_conflict << std::endl; }

        if (current_solution_ - best_solution_ > rt) {
            std::cerr << "重启" << std::endl;
            // 清空禁忌表
            tabu_list_.clear_tabu();
            // 使用历史最优解替换当前解
            current_solution_ = best_solution_;
            evaluator_        = Evaluator{latin_square, current_solution_};
            // 重启后需要重新计算冲突节点集合
            set_row_conflict_grid_(current_solution_);
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

    // 搜索结束，输出总时间
    auto end_time                         = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::clog << "搜索结束，总时间: " << std::fixed << std::setprecision(3) << elapsed.count() << " s" << std::endl;
    std::clog << "最终冲突数: " << best_solution_.total_conflict << std::endl;
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

                if (is_tabu(move, current_solution_.total_conflict + move_delta1)) {
                    // 禁忌移动
                    if (move_delta1 < best_tabu_move_delta1) {
                        // 一级评估函数更优，计算二级评估函数
                        auto move_delta2      = evaluator_.evaluate_domain_delta(current_solution_, move);
                        best_tabu_move_delta1 = move_delta1;
                        best_tabu_move_delta2 = move_delta2;
                        best_tabu_move        = move;
                        tabu_move_num         = 1;
                    } else if (move_delta1 == best_tabu_move_delta1) {
                        // 一级评估函数相同，计算二级评估函数进行比较
                        auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
                        if (move_delta2 < best_tabu_move_delta2) {
                            best_tabu_move_delta2 = move_delta2;
                            best_tabu_move        = move;
                            tabu_move_num         = 1;
                        } else if (move_delta2 == best_tabu_move_delta2) {
                            tabu_move_num++;
                            if (randomInt(tabu_move_num) == 0) { best_tabu_move = move; }
                        }
                    }
                } else {
                    // 非禁忌移动
                    if (move_delta1 < best_non_tabu_move_delta1) {
                        // 一级评估函数更优，计算二级评估函数
                        auto move_delta2          = evaluator_.evaluate_domain_delta(current_solution_, move);
                        best_non_tabu_move_delta1 = move_delta1;
                        best_non_tabu_move_delta2 = move_delta2;
                        best_non_tabu_move        = move;
                        non_tabu_move_num         = 1;
                    } else if (move_delta1 == best_non_tabu_move_delta1) {
                        // 一级评估函数相同，计算二级评估函数进行比较
                        auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
                        if (move_delta2 < best_non_tabu_move_delta2) {
                            best_non_tabu_move_delta2 = move_delta2;
                            best_non_tabu_move        = move;
                            non_tabu_move_num         = 1;
                        } else if (move_delta2 == best_non_tabu_move_delta2) {
                            non_tabu_move_num++;
                            if (randomInt(non_tabu_move_num) == 0) { best_non_tabu_move = move; }
                        }
                    }
                }
            }
        }

        // 冲突节点 - 非冲突节点
        for (const auto col1: row_conflict_grid_[row]) {
            for (const auto col2: row_nonconflict_grid_[row]) {
                Move move{row, col1, col2};

                auto move_delta1 = evaluator_.evaluate_conflict_delta(current_solution_, move);

                if (is_tabu(move, current_solution_.total_conflict + move_delta1)) {
                    // 禁忌移动
                    if (move_delta1 < best_tabu_move_delta1) {
                        // 一级评估函数更优，计算二级评估函数
                        auto move_delta2      = evaluator_.evaluate_domain_delta(current_solution_, move);
                        best_tabu_move_delta1 = move_delta1;
                        best_tabu_move_delta2 = move_delta2;
                        best_tabu_move        = move;
                        tabu_move_num         = 1;
                    } else if (move_delta1 == best_tabu_move_delta1) {
                        // 一级评估函数相同，计算二级评估函数进行比较
                        auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
                        if (move_delta2 < best_tabu_move_delta2) {
                            best_tabu_move_delta2 = move_delta2;
                            best_tabu_move        = move;
                            tabu_move_num         = 1;
                        } else if (move_delta2 == best_tabu_move_delta2) {
                            tabu_move_num++;
                            if (randomInt(tabu_move_num) == 0) { best_tabu_move = move; }
                        }
                    }
                } else {
                    // 非禁忌移动
                    if (move_delta1 < best_non_tabu_move_delta1) {
                        // 一级评估函数更优，计算二级评估函数
                        auto move_delta2          = evaluator_.evaluate_domain_delta(current_solution_, move);
                        best_non_tabu_move_delta1 = move_delta1;
                        best_non_tabu_move_delta2 = move_delta2;
                        best_non_tabu_move        = move;
                        non_tabu_move_num         = 1;
                    } else if (move_delta1 == best_non_tabu_move_delta1) {
                        // 一级评估函数相同，计算二级评估函数进行比较
                        auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
                        if (move_delta2 < best_non_tabu_move_delta2) {
                            best_non_tabu_move_delta2 = move_delta2;
                            best_non_tabu_move        = move;
                            non_tabu_move_num         = 1;
                        } else if (move_delta2 == best_non_tabu_move_delta2) {
                            non_tabu_move_num++;
                            if (randomInt(non_tabu_move_num) == 0) { best_non_tabu_move = move; }
                        }
                    }
                }
            }
        }
    }

    // 选择最佳移动：特赦规则 - 如果禁忌移动比历史最优解更好，则选择禁忌移动
    if (current_solution_.total_conflict + best_tabu_move_delta1 < best_solution_.total_conflict &&
        best_tabu_move_delta1 < best_non_tabu_move_delta1) {
        return best_tabu_move;
    }

    // 检查是否找到有效的移动
    if (best_non_tabu_move.row_id == -1) {
        throw std::runtime_error("No valid move found in find_move()");
    }

    return best_non_tabu_move;
}

void LocalSearch::make_move(const Move &move) {
    auto move_delta1 = evaluator_.evaluate_conflict_delta(current_solution_, move);
    auto move_delta2 = evaluator_.evaluate_domain_delta(current_solution_, move);
    set_tabu(move);
    evaluator_.color_in_domain_table_.make_move(current_solution_, move);
    auto affected_cells = evaluator_.col_color_num_table_.make_move(current_solution_, move);
    current_solution_.total_conflict += move_delta1;
    current_solution_.domain_conflict += move_delta2;
    std::swap(current_solution_.solution[move.row_id][move.col1], current_solution_.solution[move.row_id][move.col2]);

    // 增量更新冲突节点集合
    update_row_conflict_grid_incremental_(affected_cells);

    // 调试：验证冲突节点集合的正确性（可通过宏控制）
#ifdef VERIFY_CONFLICT_GRID
    verify_conflict_grid();
#endif
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

void LocalSearch::update_row_conflict_grid_incremental_(const std::vector<ColColorNumTable::AffectedCell> &affected_cells) {
    // 收集所有受影响的列（去重）
    std::vector<bool> affected_cols_set(current_solution_.solution.size(), false);
    for (const auto &cell: affected_cells) {
        affected_cols_set[cell.col] = true;
    }

    const int N = static_cast<int>(current_solution_.solution.size());

    // 对每个受影响的列，更新该列所有行的冲突状态
    for (int col = 0; col < N; ++col) {
        if (!affected_cols_set[col]) continue;

        // 遍历该列的所有行
        for (int row = 0; row < N; ++row) {
            // 跳过固定的格子
            if (evaluator_.color_in_domain_table_.latin_square_.is_fixed(row, col)) {
                continue;
            }

            // 获取该格子的颜色
            const auto color = current_solution_.get_color(row, col);

            // 判断该格子是否冲突
            const bool is_conflict = evaluator_.is_conflict_grid(color, col);

            // 检查之前的状态
            const bool was_in_conflict = row_conflict_grid_[row].contains(col);

            if (is_conflict && !was_in_conflict) {
                // 从非冲突变为冲突
                row_nonconflict_grid_[row].erase(col);
                row_conflict_grid_[row].insert(col);
            } else if (!is_conflict && was_in_conflict) {
                // 从冲突变为非冲突
                row_conflict_grid_[row].erase(col);
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
    constexpr double alpha = 0.4;
    // 确保禁忌期至少为10，避免冲突数过小时禁忌期过短
    const auto base_tenure                     = static_cast<unsigned long long>(alpha * current_solution_.total_conflict);
    const auto target_iteration_without_random = base_tenure + iteration_;

    // 只有当操作单元是冲突节点时，才将其加入禁忌表
    if (evaluator_.is_conflict_grid(color1, move.col1))
        tabu_list_.make_tabu(move.row_id, move.col1, color1, target_iteration_without_random + randomInt(10));
    if (evaluator_.is_conflict_grid(color2, move.col2))
        tabu_list_.make_tabu(move.row_id, move.col2, color2, target_iteration_without_random + randomInt(10));
}

void LocalSearch::verify_conflict_grid() const {
    const int N = static_cast<int>(current_solution_.solution.size());

    // 重新计算期望的冲突节点集合
    std::vector<VecSet> expected_conflict(N, VecSet{N});
    std::vector<VecSet> expected_nonconflict(N, VecSet{N});

    for (auto row = 0; row < N; ++row) {
        for (auto col = 0; col < N; ++col) {
            if (evaluator_.color_in_domain_table_.latin_square_.is_fixed(row, col)) {
                continue;
            }
            if (evaluator_.is_conflict_grid(current_solution_.get_color(row, col), col)) {
                expected_conflict[row].insert(col);
            } else {
                expected_nonconflict[row].insert(col);
            }
        }
    }

    // 验证每一行
    for (auto row = 0; row < N; ++row) {
        if (row_conflict_grid_[row] != expected_conflict[row]) {
            std::cerr << "Row " << row << " conflict grid mismatch!" << std::endl;
            std::cerr << "Expected conflict: ";
            for (auto col: expected_conflict[row]) {
                std::cerr << col << " ";
            }
            std::cerr << std::endl;
            std::cerr << "Actual conflict: ";
            for (auto col: row_conflict_grid_[row]) {
                std::cerr << col << " ";
            }
            std::cerr << std::endl;
            throw std::runtime_error("Conflict grid verification failed");
        }

        if (row_nonconflict_grid_[row] != expected_nonconflict[row]) {
            std::cerr << "Row " << row << " non-conflict grid mismatch!" << std::endl;
            throw std::runtime_error("Non-conflict grid verification failed");
        }
    }
}
}// namespace qm::latin_square