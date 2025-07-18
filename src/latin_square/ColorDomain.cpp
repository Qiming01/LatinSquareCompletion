//
// Created by qiming on 25-7-17.
//
#include <unordered_set>
#include "latin_square/ColorDomain.h"
#include "utils/RandomGenerator.h"

namespace qm::latin_square {

    // 对拉丁方颜色域进行化简
    // 约简规则一：同一行（列）内k个顶点所能使用的颜色的并集刚好是k种颜色，那么这k种颜色就不能被该行（列）内其他顶点所使用
    // 约简规则二：某k种颜色只有同一行内的k个顶点的颜色域含有，那么这k个顶点只能染这k种颜色
    // 简化的规则：k = n - 1 / 1
    void ColorDomain::simplify(SimplifyMethod method) {
        bool changed = true;
        int iteration = 0;
        const int max_iterations = 10000;
        while (changed && iteration < max_iterations) {
            changed = false;
            ++iteration;
            // 处理已固定的单元格
            if (propagateFixedValues()) {
                changed = true;
            }

            // 对行和列应用约简规则(简化的)
            if (applyReductionRulesSimply()) {
                changed = true;
            }
        }
    }


    // 传播已固定值的约束
    bool ColorDomain::propagateFixedValues(bool col_needed) {
        bool changed = false;

        for (int row = 0; row < n_; ++row) {
            for (int col = 0; col < n_; ++col) {
                if (fixed(row, col) && fixed_[row][col] == -1) {
                    int fixed_value = domains_[row][col].getFirstElement();
                    try_fix(row, col, fixed_value, col_needed);
                    changed = true;
                }
            }
        }

        return changed;
    }


    bool ColorDomain::applyReductionRulesSimply(bool col_needed) {
        bool changed = false;
        Domain<MAX_SET_SIZE> union_set(n_, InitMode::ALL_ZEROS);
        // 如果n-1个结点的并集大小为n-1，则剩余结点只能染剩下的那1种颜色。
        // 对于每一行
        for (int row = 0; row < n_; ++row) {
            for (int i = 0; i < n_; ++i) {
                if (fixed(row, i)) {
                    continue;
                }
                // 求剩余的集合的并集大小
                union_set.clear();
                for (int c = 0; c < n_; ++c) {
                    if (c != i) {
                        union_set |= domains_[row][c];
                    }
                }
                auto complement = ~union_set;
                if (complement.size == 1) {
                    // 剩余的集合的并集大小为n-1，则剩余结点只能染剩下的那1种颜色
                    auto value = complement.getFirstElement();
                    assert(domains_[row][i].contains(value));
                    try_fix(row, i, value, col_needed);
                    changed = true;
                }
            }
        }
        if (col_needed) {
            for (int col = 0; col < n_; ++col) {
                for (int i = 0; i < n_; ++i) {
                    if (fixed(i, col)) {
                        continue;
                    }
                    union_set.clear();
                    for (int r = 0; r < n_; ++r) {
                        if (r != i) {
                            union_set |= domains_[r][col];
                        }
                    }
                    auto completion = ~union_set;
                    if (completion.size == 1) {
                        auto value = completion.getFirstElement();
                        assert(domains_[i][col].contains(value));
                        try_fix(i, col, value);
                        changed = true;
                    }
                }
            }
        }
        return changed;


    }

    void ColorDomain::try_fix(int i, int j, int value, bool col_needed) {
        fixed_[i][j] = value;
        for (int col = 0; col < n_; col++) {
            domains_[i][col].remove(value);
        }
        if (col_needed) {
            for (int row = 0; row < n_; row++) {
                domains_[row][j].remove(value);
            }
        }
        domains_[i][j].clear();
        domains_[i][j].insert(value);
        ++fixed_num_;
    }

    // 获取初始解
    std::vector<std::vector<int>> ColorDomain::getInitialSolution() {
        simplify();
        // 备份颜色域和固定值
        auto domains_bk = domains_;
        auto fixed_bk = fixed_;
        auto fixed_num_bk = fixed_num_;

        std::vector<bool> row_all_fixed(n_, false);

        while (fixed_num() < n_ * n_) {
            // 对于每一行，尝试固定一个颜色
            for (int i = 0; i < n_; ++i) {
                if (row_all_fixed[i]) continue;
                int min_color_num = INT32_MAX;
                int index = -1;
                for (int j = 0; j < n_; ++j) {
                    if (domains_[i][j].size > 1 && domains_[i][j].size < min_color_num) {
                        min_color_num = domains_[i][j].size;
                        index = j;
                    }
                }
                if (index == -1) {
                    row_all_fixed[i] = true;
                    continue;
                }
                int j = index;
                // 尝试固定一个颜色
                auto value_count = domains_[i][j].size;
                if (value_count < 1) {
                    throw std::runtime_error("No more values to fix.");
                }
                auto value = domains_[i][j].getIthElement(randomInt(value_count));
                // 仅仅约简同行的颜色域
                try_fix(i, j, value, false);
                bool changed = true;

                while (changed) {
                    changed = false;
                    // 处理已固定的单元格
                    if (propagateFixedValues(false)) {
                        changed = true;
                    }

                    // 对行和列应用约简规则(简化的)
                    if (applyReductionRulesSimply(false)) {
                        changed = true;
                    }
                }

            }
        }

        const auto &solution = fixed_;
        // 检查结果中有没有行冲突
        for (int i = 0; i < n_; ++i) {
            std::unordered_set<int> row_set;
            for (int j = 0; j < n_; ++j) {
                if (solution[i][j] == -1) {
                    throw std::runtime_error("未完全初始化");
                }
                if (row_set.contains(solution[i][j])) {
                    throw std::runtime_error("行冲突");
                }
                if (solution[i][j] >= n_ || solution[i][j] < 0) {
                    throw std::runtime_error("不合法");
                }
                row_set.insert(solution[i][j]);
            }
        }
        domains_ = domains_bk;
        fixed_num_ = fixed_num_bk;
        std::swap(fixed_bk, fixed_);
        return fixed_bk;
    }
}