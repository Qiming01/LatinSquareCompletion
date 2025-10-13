//
// Created by qiming on 2025/9/10.
//

#ifndef LATINSQUARECOMPLETION_DOMAIN_H
#define LATINSQUARECOMPLETION_DOMAIN_H

#include <bitset>
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <bit>

namespace qm::latin_square {
// 颜色域初始化方式枚举
enum class InitMode {
    ALL_ONES, // 全1初始化（所有值都可用）
    ALL_ZEROS,// 全0初始化（所有值都不可用）
};

/**
 * @brief 查找bitset中第一个被设置的位（1）
 * @tparam N bitset的大小
 * @param bs 要搜索的bitset
 * @return 第一个被设置的位的索引，如果未找到则返回bs.size()
 */
template<size_t N>
size_t find_first(const std::bitset<N> &bs) {
    constexpr size_t chunk_size = 64;                               // 每次处理64位（一个uint64_t）
    constexpr size_t num_chunks = (N + chunk_size - 1) / chunk_size;// 计算需要的块数
    for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
        const size_t start = chunk * chunk_size;
        const size_t end   = std::min(start + chunk_size, N);
        // 构建当前64位块的值
        uint64_t chunk_value = 0;
        for (size_t i = start; i < end; ++i) { if (bs.test(i)) { chunk_value |= (1ULL << (i - start)); } }
        // 如果块值不为0，找到最低位的1
        if (chunk_value != 0) { return start + std::countr_zero(chunk_value); }
    }
    return bs.size();// 未找到任何设置的位
}

/**
 * @brief 查找bitset中第一个未被设置的位（0）
 * @tparam N bitset的大小
 * @param bs 要搜索的bitset
 * @return 第一个未被设置的位的索引，如果未找到则返回bs.size()
 */
template<size_t N>
size_t find_first_zero(const std::bitset<N> &bs) {
    constexpr size_t chunk_size = 64;
    constexpr size_t num_chunks = (N + chunk_size - 1) / chunk_size;

    for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
        size_t start = chunk * chunk_size;
        size_t end   = std::min(start + chunk_size, N);

        // 构建当前块的值
        uint64_t chunk_value = 0;
        for (size_t i = start; i < end; ++i) { if (bs.test(i)) { chunk_value |= (1ULL << (i - start)); } }

        // 关键差异：对chunk_value取反，然后只保留有效位
        size_t valid_bits       = end - start;
        uint64_t mask           = (valid_bits == 64) ? ~0ULL : (1ULL << valid_bits) - 1;
        uint64_t inverted_chunk = (~chunk_value) & mask;

        if (inverted_chunk != 0) { return start + std::countr_zero(inverted_chunk); }
    }
    return bs.size();// 没找到0，返回size表示未找到
}

/**
 * @brief 查找bitset中第i个被设置的位
 * @tparam N bitset的大小
 * @param bs 要搜索的bitset
 * @param i 要查找的第几个被设置的位（从0开始）
 * @return 第i个被设置的位的索引，如果不存在则返回-1
 */
template<size_t N>
int find_ith_set_bit(const std::bitset<N> &bs, size_t i) {
    size_t count = 0;
    for (size_t index = 0; index < N; ++index) {
        if (bs.test(index)) {
            if (count == i) { return static_cast<int>(index); }
            ++count;
        }
    }
    return -1;// 未找到第i个设置的位
}

/**
 * @brief 域（Domain）类模板，用于表示有限域上的值集合
 * @tparam MAX_SIZE 域的最大容量，默认为128
 */
template<size_t MAX_SIZE = 128>
struct Domain {
    std::bitset<MAX_SIZE> bits;// 使用bitset存储域中的值
    int capacity{0};           // 域的容量（最大值范围）
    int size{0};               // 当前域中实际包含的值的数量

    Domain() = default;// 默认构造函数

    /**
     * @brief 初始化域
     * @param n 域的容量
     * @param mode 初始化模式
     */
    void init(const int n, const InitMode mode = InitMode::ALL_ONES) {
        assert(n <= static_cast<int>(MAX_SIZE) && n >= 0);
        capacity = n;
        switch (mode) {
            case InitMode::ALL_ONES:
                bits.set();
                bits >>= MAX_SIZE - n;
                size = n;
                break;
            case InitMode::ALL_ZEROS:
                bits.reset();
                size = 0;
                break;
            default:
                bits.reset();
                for (int i = 0; i < n; ++i) { bits.set(i); }
                size = n;
                break;
        }
    }

    /**
     * @brief 构造函数
     * @param n 域的容量
     * @param mode 初始化模式
     */
    explicit Domain(int n, const InitMode mode = InitMode::ALL_ONES) { init(n, mode); }

    /**
     * @brief 更新size成员变量以匹配bitset中实际设置的数量
     */
    void update_size() { size = static_cast<int>(bits.count()); }

    // 拷贝构造函数和移动构造函数
    Domain(const Domain &)            = default;
    Domain(Domain &&)                 = default;
    Domain &operator=(Domain &&)      = default;
    Domain &operator=(const Domain &) = default;

    /**
     * @brief 并集运算
     * @param other 另一个域
     * @return 两个域的并集
     */
    Domain operator|(const Domain &other) const {
        Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
        result.bits = bits | other.bits;
        result.update_size();
        return result;
    }

    /**
    * @brief 并集赋值运算
    * @param other 另一个域
    * @return 当前域的引用
    */
    Domain &operator|=(const Domain &other) {
        if (other.capacity > capacity) capacity = other.capacity;
        bits |= other.bits;
        update_size();
        return *this;
    }

    /**
     * @brief 交集运算
     * @param other 另一个域
     * @return 两个域的交集
     */
    Domain operator&(const Domain &other) const {
        Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
        result.bits = bits & other.bits;
        result.update_size();
        return result;
    }

    /**
     * @brief 交集赋值运算
     * @param other 另一个域
     * @return 当前域的引用
     */
    Domain &operator&=(const Domain &other) {
        if (other.capacity > capacity) capacity = other.capacity;
        bits &= other.bits;
        update_size();
        return *this;
    }

    /**
     * @brief 差集运算
     * @param other 另一个域
     * @return 当前域减去另一个域的结果
     */
    Domain operator-(const Domain &other) const {
        Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
        result.bits = bits & (~other.bits);
        result.update_size();
        return result;
    }

    /**
     * @brief 差集赋值运算
     * @param other 另一个域
     * @return 当前域的引用
     */
    Domain &operator-=(const Domain &other) {
        if (other.capacity > capacity) capacity = other.capacity;
        bits &= (~other.bits);
        update_size();
        return *this;
    }

    /**
     * @brief 补集运算
     * @return 当前域的补集
     */
    Domain operator~() const {
        Domain result(capacity, InitMode::ALL_ZEROS);
        result.bits = ~bits;
        for (int i = capacity; i < static_cast<int>(MAX_SIZE); ++i) { result.bits.reset(i); }
        result.update_size();
        return result;
    }

    /**
     * @brief 尝试计算并集的大小
     * @param other 另一个域
     * @return 并集的大小
     */
    [[nodiscard]] int try_union(const Domain &other) const { return static_cast<int>((bits | other.bits).count()); }

    /**
     * @brief 尝试计算交集的大小
     * @param other 另一个域
     * @return 交集的大小
     */
    [[nodiscard]] int try_intersection(const Domain &other) const { return static_cast<int>((bits & other.bits).count()); }

    /**
     * @brief 尝试计算差集的大小
     * @param other 另一个域
     * @return 差集的大小
     */
    [[nodiscard]] int try_subtraction(const Domain &other) const { return static_cast<int>((bits & (~other.bits)).count()); }

    /**
     * @brief 尝试计算补集的大小
     * @return 补集的大小
     */
    [[nodiscard]] int try_complement() const {
        std::bitset<MAX_SIZE> temp = ~bits;
        for (int i = capacity; i < static_cast<int>(MAX_SIZE); ++i) { temp.reset(i); }
        return static_cast<int>(temp.count());
    }

    /**
     * @brief 将域转换为向量
     * @return 包含域中所有值的向量
     */
    [[nodiscard]] std::vector<int> to_vector() const {
        std::vector<int> result;
        result.reserve(size);
        for (int i = 0; i < capacity; ++i) { if (bits.test(i)) { result.push_back(i); } }
        return result;
    }

    /**
     * @brief 从向量创建域
     * @param vec 包含域值的向量
     * @param cap 域的容量
     * @return 新创建的域
     */
    static Domain from_vector(const std::vector<int> &vec, int cap) {
        Domain result(cap, InitMode::ALL_ZEROS);
        for (int val: vec) { if (val >= 0 && val < cap) { result.bits.set(val); } }
        result.update_size();
        return result;
    }

    /**
     * @brief 检查域是否为空
     * @return 如果域为空返回true，否则返回false
     */
    [[nodiscard]] bool empty() const { return size == 0; }

    /**
     * @brief 检查域是否已满（包含所有可能的值）
     * @return 如果域已满返回true，否则返回false
     */
    [[nodiscard]] bool full() const { return size == capacity; }

    /**
     * @brief 获取域的大小
     * @return 域中值的数量
     */
    [[nodiscard]] int get_size() const { return size; }

    /**
     * @brief 获取域的容量
     * @return 域的最大容量
     */
    [[nodiscard]] int get_capacity() const { return capacity; }

    /**
     * @brief 检查域是否包含某个值
     * @param value 要检查的值
     * @return 如果包含该值返回true，否则返回false
     */
    [[nodiscard]] bool contains(int value) const { return value >= 0 && value < capacity && bits.test(value); }

    /**
     * @brief 向域中插入一个值
     * @param value 要插入的值
     */
    void insert(int value) {
        if (value >= 0 && value < capacity && !bits.test(value)) {
            bits.set(value);
            ++size;
        }
    }

    /**
     * @brief 从域中移除一个值
     * @param value 要移除的值
     */
    void remove(int value) {
        if (value >= 0 && value < capacity && bits.test(value)) {
            bits.reset(value);
            --size;
        }
    }

    /**
     * @brief 清空域
     */
    void clear() {
        bits.reset();
        size = 0;
    }

    /**
     * @brief 获取第i个被设置的值
     * @param i 索引（从0开始）
     * @return 第i个被设置的值，如果不存在返回-1
     */
    [[nodiscard]] int get_ith_element(size_t i) const { return find_ith_set_bit(bits, i); }

    /**
    * @brief 获取第一个被设置的值
    * @return 第一个被设置的值，如果域为空返回-1
    */
    [[nodiscard]] int get_first_element() const {
        if (size == 0) return -1;
        return static_cast<int>(find_first(bits));
    }

    /**
     * @brief 获取第一个未被设置的值
     * @return 第一个未被设置的值，如果域已满返回-1
     */
    [[nodiscard]] int get_first_zero() const {
        if (size == 0) return -1;
        return static_cast<int>(Find_first_zero(bits));
    }

    /**
     * @brief 将域转换为字符串表示
     * @return 域的字符串表示
     */
    [[nodiscard]] std::string to_string() const {
        std::string result = "Domain[";
        bool first         = true;
        for (int i = 0; i < capacity; ++i) {
            if (bits.test(i)) {
                if (!first) result += ",";
                result += std::to_string(i);
                first = false;
            }
        }
        result += "] size=" + std::to_string(size) + "/" + std::to_string(capacity);
        return result;
    }

    /**
     * @brief 相等比较运算符
     * @param other 另一个域
     * @return 如果两个域相等返回true，否则返回false
     */
    bool operator==(const Domain &other) const { return capacity == other.capacity && bits == other.bits; }

    /**
     * @brief 不等比较运算符
     * @param other 另一个域
     * @return 如果两个域不相等返回true，否则返回false
     */
    bool operator!=(const Domain &other) const { return !(*this == other); }

    /**
     * @brief 检查当前域是否是另一个域的子集
     * @param other 另一个域
     * @return 如果是子集返回true，否则返回false
     */
    [[nodiscard]] bool is_subset_of(const Domain &other) const { return (bits & other.bits) == bits; }

    /**
     * @brief 检查当前域是否是另一个域的超集
     * @param other 另一个域
     * @return 如果是超集返回true，否则返回false
     */
    [[nodiscard]] bool is_superset_of(const Domain &other) const { return other.is_superset_of(*this); }
};

// 静态断言确保Domain类是可拷贝和移动的
static_assert(std::is_copy_constructible_v<Domain<>>, "Domain must be copyable");
static_assert(std::is_move_constructible_v<Domain<>>, "Domain must be movable");
}
#endif //LATINSQUARECOMPLETION_DOMAIN_H