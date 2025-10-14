//
// Created by qiming on 2025/10/14.
//

/**
 * @file vec_set.h
 * @brief 基于向量实现的快速集合类，支持O(1)时间的插入、删除和查询操作
 *
 * 这个类使用两个向量来维护集合：
 * - data_: 存储集合中的实际元素（无序）
 * - pos_: 记录每个元素在data_中的位置，用于快速查找
 *
 * 适用于元素ID在固定范围内的场景，支持高效的集合运算。
 */

#ifndef VEC_SET_H
#define VEC_SET_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>

/**
 * @class VecSet
 * @brief 基于向量的快速集合实现
 *
 * 支持O(1)时间的插入、删除和包含判断，以及高效的集合运算（交集、并集、差集等）。
 * 要求所有元素必须在[0, universe_size)范围内。
 */
class VecSet {
public:
    // 迭代器类型定义
    using iterator               = std::vector<int>::iterator;              ///< 正向迭代器
    using const_iterator         = std::vector<int>::const_iterator;        ///< 常量正向迭代器
    using reverse_iterator       = std::vector<int>::reverse_iterator;      ///< 反向迭代器
    using const_reverse_iterator = std::vector<int>::const_reverse_iterator;///< 常量反向迭代器

    /**
     * @brief 构造函数
     * @param universe_size 全集大小，元素ID范围为[0, universe_size)
     */
    explicit VecSet(int universe_size = 0)
        : pos_(universe_size, -1) {
        data_.reserve(std::min(universe_size, 64));// 预分配合理容量
    }

    /**
     * @brief 移动构造函数
     */
    VecSet(VecSet &&) noexcept = default;

    /**
     * @brief 移动赋值运算符
     */
    VecSet &operator=(VecSet &&) noexcept = default;

    /**
     * @brief 拷贝构造函数
     */
    VecSet(const VecSet &) = default;

    /**
     * @brief 拷贝赋值运算符
     */
    VecSet &operator=(const VecSet &) = default;

    /**
     * @brief 获取全集大小
     * @return 全集大小
     */
    int universe_size() const noexcept { return (int) pos_.size(); }

    /**
     * @brief 获取集合中元素个数
     * @return 集合大小
     */
    int size() const noexcept { return (int) data_.size(); }

    /**
     * @brief 判断集合是否为空
     * @return 如果集合为空返回true，否则返回false
     */
    bool empty() const noexcept { return data_.empty(); }

    /**
     * @brief 预留存储空间
     * @param capacity 期望的容量
     */
    void reserve(int capacity) { data_.reserve(capacity); }

    /**
     * @brief 清空集合
     *
     * 时间复杂度：O(|S|)，其中|S|是集合大小
     */
    void clear() noexcept {
        for (int x: data_) pos_[x] = -1;
        data_.clear();
    }

    /**
     * @brief 判断集合是否包含指定元素
     * @param x 要查找的元素
     * @return 如果包含返回true，否则返回false
     *
     * 时间复杂度：O(1)
     */
    bool contains(int x) const noexcept { return (unsigned) x < (unsigned) pos_.size() && pos_[x] != -1; }

    /**
     * @brief 向集合中插入元素
     * @param x 要插入的元素
     * @return 如果插入成功（元素原本不存在）返回true，否则返回false
     *
     * 均摊时间复杂度：O(1)
     * @throw std::out_of_range 如果元素超出全集范围
     */
    bool insert(int x) {
        check_id(x);
        if (pos_[x] != -1) return false;
        pos_[x] = (int) data_.size();
        data_.push_back(x);
        return true;
    }

    /**
     * @brief 从集合中删除元素
     * @param x 要删除的元素
     * @return 如果删除成功（元素原本存在）返回true，否则返回false
     *
     * 均摊时间复杂度：O(1)
     */
    bool erase(int x) noexcept {
        if ((unsigned) x >= (unsigned) pos_.size()) return false;
        int i = pos_[x];
        if (i == -1) return false;

        int last   = data_.back();
        data_[i]   = last;
        pos_[last] = i;
        data_.pop_back();
        pos_[x] = -1;
        return true;
    }

    /**
     * @brief 通过下标访问集合元素
     * @param idx 下标位置
     * @return 对应位置的元素
     *
     * 时间复杂度：O(1)
     * 注意：元素在data_中的存储是无序的
     */
    int operator[](int idx) const noexcept { return data_[idx]; }

    /**
     * @brief 通过下标安全访问集合元素
     * @param idx 下标位置
     * @return 对应位置的元素
     * @throw std::out_of_range 如果下标越界
     */
    int at(int idx) const { return data_.at(idx); }

    /**
     * @brief 获取集合中所有元素的只读引用
     * @return 包含所有元素的常量向量引用
     */
    const std::vector<int> &elements() const noexcept { return data_; }

    // ==== 迭代器接口 ====

    iterator begin() noexcept { return data_.begin(); }              ///< 返回指向第一个元素的正向迭代器
    iterator end() noexcept { return data_.end(); }                  ///< 返回指向末尾的正向迭代器
    const_iterator begin() const noexcept { return data_.begin(); }  ///< 返回指向第一个元素的常量正向迭代器
    const_iterator end() const noexcept { return data_.end(); }      ///< 返回指向末尾的常量正向迭代器
    const_iterator cbegin() const noexcept { return data_.cbegin(); }///< 返回指向第一个元素的常量正向迭代器
    const_iterator cend() const noexcept { return data_.cend(); }    ///< 返回指向末尾的常量正向迭代器

    reverse_iterator rbegin() noexcept { return data_.rbegin(); }              ///< 返回指向最后一个元素的反向迭代器
    reverse_iterator rend() noexcept { return data_.rend(); }                  ///< 返回指向开头前一个位置的反向迭代器
    const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }  ///< 返回指向最后一个元素的常量反向迭代器
    const_reverse_iterator rend() const noexcept { return data_.rend(); }      ///< 返回指向开头前一个位置的常量反向迭代器
    const_reverse_iterator crbegin() const noexcept { return data_.crbegin(); }///< 返回指向最后一个元素的常量反向迭代器
    const_reverse_iterator crend() const noexcept { return data_.crend(); }    ///< 返回指向开头前一个位置的常量反向迭代器

    // ==== 集合运算：要求同一全集大小 ====

    /**
     * @brief 计算两个集合的交集
     * @param A 第一个集合
     * @param B 第二个集合
     * @return 交集结果
     *
     * 时间复杂度：O(min(|A|,|B|))
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    static VecSet intersection(const VecSet &A, const VecSet &B) {
        ensure_same_universe(A, B);
        const VecSet &small = (A.size() <= B.size()) ? A : B;
        const VecSet &large = (&small == &A) ? B : A;

        VecSet C(A.universe_size());
        C.reserve(small.size());

        for (int x: small.data_) { if ((unsigned) x < (unsigned) large.pos_.size() && large.pos_[x] != -1) { C._push_known_absent(x); } }
        return C;
    }

    /**
     * @brief 计算两个集合的并集
     * @param A 第一个集合
     * @param B 第二个集合
     * @return 并集结果
     *
     * 时间复杂度：O(|A|+|B|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    static VecSet union_set(const VecSet &A, const VecSet &B) {
        ensure_same_universe(A, B);
        VecSet C(A.universe_size());
        C.reserve(A.size() + B.size());

        C.data_ = A.data_;
        for (size_t i = 0; i < A.data_.size(); ++i) { C.pos_[A.data_[i]] = (int) i; }

        for (int x: B.data_) { if (C.pos_[x] == -1) { C._push_known_absent(x); } }
        return C;
    }

    /**
     * @brief 计算两个集合的差集 A \ B
     * @param A 第一个集合
     * @param B 第二个集合
     * @return 差集结果
     *
     * 时间复杂度：O(|A|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    static VecSet difference(const VecSet &A, const VecSet &B) {
        ensure_same_universe(A, B);
        if (&A == &B) return VecSet(A.universe_size());

        VecSet C(A.universe_size());
        C.reserve(A.size());

        for (int x: A.data_) { if ((unsigned) x >= (unsigned) B.pos_.size() || B.pos_[x] == -1) { C._push_known_absent(x); } }
        return C;
    }

    /**
     * @brief 计算集合的补集
     * @return 补集结果
     *
     * 时间复杂度：O(U)，其中U是全集大小
     */
    VecSet complement() const {
        VecSet C(universe_size());
        C.reserve(universe_size() - size());

        for (int x = 0, U = universe_size(); x < U; ++x) { if (pos_[x] == -1) C._push_known_absent(x); }
        return C;
    }

    /**
     * @brief 计算两个集合的对称差 (A △ B = (A \ B) ∪ (B \ A))
     * @param A 第一个集合
     * @param B 第二个集合
     * @return 对称差结果
     *
     * 时间复杂度：O(|A|+|B|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    static VecSet symmetric_difference(const VecSet &A, const VecSet &B) {
        ensure_same_universe(A, B);
        VecSet C(A.universe_size());
        C.reserve(A.size() + B.size());

        for (int x: A.data_) { if ((unsigned) x >= (unsigned) B.pos_.size() || B.pos_[x] == -1) { C._push_known_absent(x); } }
        for (int x: B.data_) { if ((unsigned) x >= (unsigned) A.pos_.size() || A.pos_[x] == -1) { C._push_known_absent(x); } }
        return C;
    }

    // ==== 原地操作（更高效）====

    /**
     * @brief 原地并集操作：this = this ∪ other
     * @param other 另一个集合
     *
     * 均摊时间复杂度：O(|other|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    void unite_with(const VecSet &other) {
        ensure_same_universe(*this, other);
        reserve(size() + other.size());
        for (int x: other.data_) { if (pos_[x] == -1) _push_known_absent(x); }
    }

    /**
     * @brief 原地交集操作：this = this ∩ other
     * @param other 另一个集合
     *
     * 时间复杂度：O(|this|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    void intersect_with(const VecSet &other) {
        ensure_same_universe(*this, other);
        int write_pos = 0;
        for (int i = 0; i < (int) data_.size(); ++i) {
            int x = data_[i];
            if ((unsigned) x < (unsigned) other.pos_.size() && other.pos_[x] != -1) {
                data_[write_pos] = x;
                pos_[x]          = write_pos;
                ++write_pos;
            } else { pos_[x] = -1; }
        }
        data_.resize(write_pos);
    }

    /**
     * @brief 原地差集操作：this = this \ other
     * @param other 另一个集合
     *
     * 时间复杂度：O(|other|)
     * @throw std::runtime_error 如果两个集合的全集大小不同
     */
    void subtract(const VecSet &other) {
        ensure_same_universe(*this, other);
        if (&other == this) {
            clear();
            return;
        }

        int write_pos = 0;
        for (int i = 0; i < (int) data_.size(); ++i) {
            int x = data_[i];
            if ((unsigned) x >= (unsigned) other.pos_.size() || other.pos_[x] == -1) {
                data_[write_pos] = x;
                pos_[x]          = write_pos;
                ++write_pos;
            } else { pos_[x] = -1; }
        }
        data_.resize(write_pos);
    }

    // ==== 关系判断 ====

    /**
     * @brief 判断当前集合是否是other的子集
     * @param other 另一个集合
     * @return 如果是子集返回true，否则返回false
     *
     * 时间复杂度：O(|this|)
     */
    bool is_subset_of(const VecSet &other) const noexcept {
        if (size() > other.size()) return false;
        for (int x: data_) { if ((unsigned) x >= (unsigned) other.pos_.size() || other.pos_[x] == -1) return false; }
        return true;
    }

    /**
     * @brief 判断两个集合是否不相交
     * @param other 另一个集合
     * @return 如果不相交返回true，否则返回false
     *
     * 时间复杂度：O(min(|this|,|other|))
     */
    bool is_disjoint(const VecSet &other) const noexcept {
        const VecSet &small = (size() <= other.size()) ? *this : other;
        const VecSet &large = (&small == this) ? other : *this;

        for (int x: small.data_) { if ((unsigned) x < (unsigned) large.pos_.size() && large.pos_[x] != -1) return false; }
        return true;
    }

    // ==== 运算符重载 ====

    /**
     * @brief 判断两个集合是否相等
     * @param other 另一个集合
     * @return 如果相等返回true，否则返回false
     */
    bool operator==(const VecSet &other) const noexcept {
        if (size() != other.size()) return false;
        for (int x: data_) { if ((unsigned) x >= (unsigned) other.pos_.size() || other.pos_[x] == -1) return false; }
        return true;
    }

    /**
     * @brief 判断两个集合是否不相等
     * @param other 另一个集合
     * @return 如果不相等返回true，否则返回false
     */
    bool operator!=(const VecSet &other) const noexcept { return !(*this == other); }

private:
    std::vector<int> data_;///< 存储集合中的实际元素
    std::vector<int> pos_; ///< 记录每个元素在data_中的位置，-1表示不存在

    /**
     * @brief 内部方法：已知元素不存在时插入元素
     * @param x 要插入的元素
     */
    void _push_known_absent(int x) noexcept {
        pos_[x] = (int) data_.size();
        data_.push_back(x);
    }

    /**
     * @brief 确保两个集合有相同的全集大小
     * @param A 第一个集合
     * @param B 第二个集合
     * @throw std::runtime_error 如果全集大小不同
     */
    static void ensure_same_universe(const VecSet &A, const VecSet &B) { if (A.universe_size() != B.universe_size()) throw std::runtime_error("VecSet universe_size mismatch"); }

    /**
     * @brief 检查元素ID是否在有效范围内
     * @param x 要检查的元素
     * @throw std::out_of_range 如果元素超出全集范围
     */
    void check_id(int x) const { if ((unsigned) x >= (unsigned) pos_.size()) throw std::out_of_range("id out of universe range"); }
};

#endif // VEC_SET_H

// ======= 使用示例 =======
/*
#include "VecSet.h"
#include <iostream>

int main() {
    VecSet A(10), B(10);
    A.insert(1); A.insert(3); A.insert(5);
    B.insert(3); B.insert(4); B.insert(9);

    // 使用迭代器
    std::cout << "A elements (using iterator): ";
    for (auto it = A.begin(); it != A.end(); ++it) {
        std::cout << *it << ' ';
    }
    std::cout << '\n';

    // 使用范围 for
    std::cout << "B elements (range-for): ";
    for (int x : B) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    // 集合运算
    auto I = VecSet::intersection(A, B);
    auto U = VecSet::union_set(A, B);
    auto D = VecSet::difference(A, B);
    auto S = VecSet::symmetric_difference(A, B);
    auto C = A.complement();

    auto print = [](const char* name, const VecSet& S) {
        std::cout << name << " (" << S.size() << "): ";
        for (int x : S) std::cout << x << ' ';
        std::cout << '\n';
    };

    print("Intersection", I);
    print("Union", U);
    print("Difference A\\B", D);
    print("Symmetric Diff", S);
    print("Complement of A", C);

    // 关系判断
    std::cout << "I is subset of A: " << (I.is_subset_of(A) ? "Yes" : "No") << '\n';
    std::cout << "A and {0,2,4} disjoint: ";
    VecSet X(10); X.insert(0); X.insert(2); X.insert(4);
    std::cout << (A.is_disjoint(X) ? "Yes" : "No") << '\n';

    // 原地操作
    VecSet Y(10);
    Y.insert(2); Y.insert(3);
    Y.unite_with(A);
    print("Y after unite with A", Y);

    Y.intersect_with(B);
    print("Y after intersect with B", Y);

    return 0;
}
*/