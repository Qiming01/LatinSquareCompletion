//
// Created by qiming on 25-7-17.
//
#include "latin_square/ColorDomain.h"

namespace qm::latin_square {

    // 对拉丁方颜色域进行化简
    // 约简规则一：同一行（列）内k个顶点所能使用的颜色的并集刚好是k种颜色，那么这k种颜色就不能被该行（列）内其他顶点所使用
    // 约简规则二：某k种颜色只有同一行内的k个顶点的颜色域含有，那么这k个顶点只能染这k种颜色
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
    bool ColorDomain::propagateFixedValues() {
        bool changed = false;

        for (int row = 0; row < n_; ++row) {
            for (int col = 0; col < n_; ++col) {
                if (fixed(row, col) && !fixed_[row][col]) {
                    int fixed_value = domains_[row][col].getFirstElement();
                    try_fix(row, col, fixed_value);
                    changed = true;
                }
            }
        }

        return changed;
    }

    int ColorDomain::fixed_num() const {
        int count = 0;
        for (int i = 0; i < n_; ++i) {
            for (int j = 0; j < n_; ++j) {
                if (fixed(i, j)) {
                    ++count;
                }
            }
        }
        return count;
    }

    bool ColorDomain::applyReductionRulesSimply() {
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
                    try_fix(row, i, value);
                    changed = true;
                }
            }
        }
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
        return changed;
    }

    void ColorDomain::try_fix(int i, int j, int value) {
        fixed_[i][j] = true;
        for (int col = 0; col < n_; col++) {
            domains_[col][j].remove(value);
        }
        for (int row = 0; row < n_; row++) {
            domains_[i][row].remove(value);
        }
        domains_[i][j].clear();
        domains_[i][j].insert(value);
    }
}