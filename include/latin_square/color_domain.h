#pragma once

#include "latin_square/domain.h"
#include <vector>


namespace qm::latin_square {

/**
 * @brief 颜色域类，用于拉丁方问题的颜色域管理
 */
class ColorDomain {
public:
    static constexpr int MAX_SET_SIZE = 100;// 最大集合大小

    /**
     * @brief 构造函数
     * @param n 拉丁方的大小（n x n）
     */
    explicit ColorDomain(int n) : n_(n) {
        const Domain<MAX_SET_SIZE> domain(n, InitMode::ALL_ONES);
        fixed_.resize(n);
        domains_.resize(n);
        for (int i = 0; i < n; ++i) {
            fixed_[i].resize(n, -1);      // 初始化固定值矩阵
            domains_[i].resize(n, domain);// 初始化域矩阵
        }
    }

    /**
     * @brief 下标运算符（获取第i行的所有域）
     * @param i 行索引
     * @return 第i行的域向量
     */
    auto operator[](int i) { return domains_[i]; }

    /**
     * @brief 函数调用运算符（获取指定位置的域，可修改）
     * @param i 行索引
     * @param j 列索引
     * @return 指定位置的域引用
     */
    Domain<MAX_SET_SIZE> &operator()(int i, int j) { return domains_[i][j]; }

    /**
     * @brief 函数调用运算符（获取指定位置的域，只读）
     * @param i 行索引
     * @param j 列索引
     * @return 指定位置的域常量引用
     */
    const Domain<MAX_SET_SIZE> &operator()(int i, int j) const { return domains_[i][j]; }

    /**
     * @brief 检查指定位置的值是否已固定
     * @param i 行索引
     * @param j 列索引
     * @return 如果值已固定返回true，否则返回false
     */
    [[nodiscard]] bool fixed(int i, int j) const { return domains_[i][j].get_size() == 1; }

    /**
     * @brief 设置指定位置为固定值
     * @param i 行索引
     * @param j 列索引
     * @param value 要设置的固定值
     */
    void set_fixed(int i, int j, int value) {
        domains_[i][j].clear();
        domains_[i][j].insert(value);
    }

    /**
     * @brief 简化颜色域
     */
    void simplify();

    /**
     * @brief 获取初始解
     * @return 初始解的二维向量
     */
    std::vector<std::vector<int>> get_initial_solution();

    /**
     * @brief 获取已固定值的数量
     * @return 已固定值的数量
     */
    [[nodiscard]] int fixed_num() const { return fixed_num_; }

    /**
     * @brief 获取总域大小
     * @return 所有域的大小总和
     */
    [[nodiscard]] int total_domain_size() const;

private:
    int n_;                                                 // 拉丁方的大小
    std::vector<std::vector<Domain<MAX_SET_SIZE>>> domains_;// 域矩阵
    std::vector<std::vector<int>> fixed_;                   // 固定值矩阵
    int fixed_num_{0};                                      // 已固定值的数量

    /**
     * @brief 应用简化规则（简单版本）
     * @param col_needed 是否需要列约束
     * @return 如果进行了简化返回true，否则返回false
     */
    bool apply_reduction_rules_simply(bool col_needed = true);

    /**
     * @brief 尝试固定一个值，并对同行同列的值删除该颜色
     * @param i 行索引
     * @param j 列索引
     * @param value 要固定的值
     * @param col_needed 是否需要列约束
     */
    void try_fix(int i, int j, int value, bool col_needed = true);

    /**
     * @brief 传播固定值的影响
     * @param col_needed 是否需要列约束
     * @return 如果成功传播返回true，否则返回false
     */
    bool propagate_fixed_values(bool col_needed = true);
};

}// namespace qm::latin_square
