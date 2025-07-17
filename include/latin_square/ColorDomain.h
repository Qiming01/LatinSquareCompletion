#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <bit>

namespace qm::latin_square {

    // 颜色域初始化方式
    enum class InitMode {
        ALL_ONES,
        ALL_ZEROS,
    };

    template<size_t N>
    size_t Find_first(const std::bitset<N> &bs) {
        constexpr size_t chunk_size = 64;
        constexpr size_t num_chunks = (N + chunk_size - 1) / chunk_size;

        for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
            size_t start = chunk * chunk_size;
            size_t end = std::min(start + chunk_size, N);

            uint64_t chunk_value = 0;
            for (size_t i = start; i < end; ++i) {
                if (bs.test(i)) {
                    chunk_value |= (1ULL << (i - start));
                }
            }

            if (chunk_value != 0) {
                return start + std::countr_zero(chunk_value);
            }
        }
        return bs.size();
    }

    template<size_t N>
    size_t Find_first_zero(const std::bitset<N> &bs) {
        constexpr size_t chunk_size = 64;
        constexpr size_t num_chunks = (N + chunk_size - 1) / chunk_size;

        for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
            size_t start = chunk * chunk_size;
            size_t end = std::min(start + chunk_size, N);

            uint64_t chunk_value = 0;
            // 构建当前块的值
            for (size_t i = start; i < end; ++i) {
                if (bs.test(i)) {
                    chunk_value |= (1ULL << (i - start));
                }
            }

            // 关键差异：对chunk_value取反，然后只保留有效位
            size_t valid_bits = end - start;
            uint64_t mask = (valid_bits == 64) ? ~0ULL : (1ULL << valid_bits) - 1;
            uint64_t inverted_chunk = (~chunk_value) & mask;

            if (inverted_chunk != 0) {
                return start + std::countr_zero(inverted_chunk);
            }
        }
        return bs.size(); // 没找到0，返回size表示未找到
    }

    template<size_t N>
    int findIthSetBit(const std::bitset<N> &bs, size_t i) {
        size_t count = 0;
        for (size_t index = 0; index < N; ++index) {
            if (bs.test(index)) {
                if (count == i) {
                    return static_cast<int>(index);
                }
                ++count;
            }
        }
        return -1;
    }

    template<size_t MAX_SIZE = 128>
    struct Domain {
        std::bitset<MAX_SIZE> bits;
        int capacity{0};
        int size{0};

        Domain() = default;

        void init(int n, InitMode mode = InitMode::ALL_ONES) {
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
                    for (int i = 0; i < n; ++i) {
                        bits.set(i);
                    }
                    size = n;
                    break;
            }
        }

        explicit Domain(int n, InitMode mode = InitMode::ALL_ONES) {
            init(n, mode);
        }

        void updateSize() {
            size = static_cast<int>(bits.count());
        }

        Domain(const Domain &) = default;

        Domain(Domain &&) = default;

        Domain &operator=(Domain &&) = default;

        Domain &operator=(const Domain &) = default;

        Domain operator|(const Domain &other) const {
            Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
            result.bits = bits | other.bits;
            result.updateSize();
            return result;
        }

        Domain &operator|=(const Domain &other) {
            if (other.capacity > capacity) capacity = other.capacity;
            bits |= other.bits;
            updateSize();
            return *this;
        }

        Domain operator&(const Domain &other) const {
            Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
            result.bits = bits & other.bits;
            result.updateSize();
            return result;
        }

        Domain &operator&=(const Domain &other) {
            if (other.capacity > capacity) capacity = other.capacity;
            bits &= other.bits;
            updateSize();
            return *this;
        }

        Domain operator-(const Domain &other) const {
            Domain result(std::max(capacity, other.capacity), InitMode::ALL_ZEROS);
            result.bits = bits & (~other.bits);
            result.updateSize();
            return result;
        }

        Domain &operator-=(const Domain &other) {
            if (other.capacity > capacity) capacity = other.capacity;
            bits &= (~other.bits);
            updateSize();
            return *this;
        }

        Domain operator~() const {
            Domain result(capacity, InitMode::ALL_ZEROS);
            result.bits = ~bits;
            for (int i = capacity; i < static_cast<int>(MAX_SIZE); ++i) {
                result.bits.reset(i);
            }
            result.updateSize();
            return result;
        }

        [[nodiscard]] int tryUnion(const Domain &other) const {
            return static_cast<int>((bits | other.bits).count());
        }

        [[nodiscard]] int tryIntersection(const Domain &other) const {
            return static_cast<int>((bits & other.bits).count());
        }

        [[nodiscard]] int trySubtraction(const Domain &other) const {
            return static_cast<int>((bits & (~other.bits)).count());
        }

        [[nodiscard]] int tryComplement() const {
            std::bitset<MAX_SIZE> temp = ~bits;
            for (int i = capacity; i < static_cast<int>(MAX_SIZE); ++i) {
                temp.reset(i);
            }
            return static_cast<int>(temp.count());
        }

        [[nodiscard]] std::vector<int> toVector() const {
            std::vector<int> result;
            result.reserve(size);
            for (int i = 0; i < capacity; ++i) {
                if (bits.test(i)) {
                    result.push_back(i);
                }
            }
            return result;
        }

        static Domain fromVector(const std::vector<int> &vec, int cap) {
            Domain result(cap, InitMode::ALL_ZEROS);
            for (int val: vec) {
                if (val >= 0 && val < cap) {
                    result.bits.set(val);
                }
            }
            result.updateSize();
            return result;
        }

        [[nodiscard]] bool empty() const { return size == 0; }

        [[nodiscard]] bool full() const { return size == capacity; }

        [[nodiscard]] int getSize() const {
            return static_cast<int>(bits.count());
        }

        [[nodiscard]] int getCapacity() const { return capacity; }

        [[nodiscard]] bool contains(int value) const {
            return value >= 0 && value < capacity && bits.test(value);
        }

        void insert(int value) {
            if (value >= 0 && value < capacity && !bits.test(value)) {
                bits.set(value);
                ++size;
            }
        }

        void remove(int value) {
            if (value >= 0 && value < capacity && bits.test(value)) {
                bits.reset(value);
                --size;
            }
        }

        void clear() {
            bits.reset();
            size = 0;
        }

        [[nodiscard]] int getIthElement(size_t i) const {
            return findIthSetBit(bits, i);
        }

        [[nodiscard]] int getFirstElement() const {
            if (size == 0) return -1;
            return static_cast<int>(Find_first(bits));
        }

        [[nodiscard]] int getFirstZero() const {
            if (size == 0) return -1;
            return static_cast<int>(Find_first_zero(bits));
        }

        [[nodiscard]] std::string toString() const {
            std::string result = "Domain[";
            bool first = true;
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

        bool operator==(const Domain &other) const {
            return capacity == other.capacity && bits == other.bits;
        }

        bool operator!=(const Domain &other) const {
            return !(*this == other);
        }

        [[nodiscard]] bool isSubsetOf(const Domain &other) const {
            return (bits & other.bits) == bits;
        }

        [[nodiscard]] bool isSupersetOf(const Domain &other) const {
            return other.isSubsetOf(*this);
        }
    };

    static_assert(std::is_copy_constructible_v<Domain<>>, "Domain must be copyable");
    static_assert(std::is_move_constructible_v<Domain<>>, "Domain must be movable");

    enum class SimplifyMethod {
        SIMPLE,
        FULL
    };

    class ColorDomain {
    public:
        static constexpr int MAX_SET_SIZE = 100;

        explicit ColorDomain(int n) : n_(n) {
            const Domain<MAX_SET_SIZE> domain(n, InitMode::ALL_ONES);
            fixed_.resize(n);
            domains_.resize(n);
            for (int i = 0; i < n; ++i) {
                fixed_[i].resize(n, false);
                domains_[i].resize(n, domain);
            }
        }

        auto operator[](int i) {
            return domains_[i];
        }

        Domain<MAX_SET_SIZE> &operator()(int i, int j) {
            return domains_[i][j];
        }

        const Domain<MAX_SET_SIZE> &operator()(int i, int j) const {
            return domains_[i][j];
        }

        [[nodiscard]] bool fixed(int i, int j) const {
            return domains_[i][j].getSize() == 1;
        }

        void set_fixed(int i, int j, int value) {
            domains_[i][j].clear();
            domains_[i][j].insert(value);
        }

        // 约简颜色域
        void simplify(SimplifyMethod method = SimplifyMethod::SIMPLE);

        // 颜色域中有多少个固定的值
        [[nodiscard]] int fixed_num() const;

    private:

        int n_;
        std::vector<std::vector<Domain<MAX_SET_SIZE>>> domains_;
        std::vector<std::vector<bool>> fixed_;


        // 简化的约简规则：如果n-1个结点的并集大小为n-1，则剩余结点只能染剩下的那1种颜色。
        bool applyReductionRulesSimply();

        // 尝试固定一个结点，并对同行同列的结点删除该颜色
        void try_fix(int i, int j, int value);

        bool applyReductionRulesForRow(int row);

        bool applyReductionRulesForColumn(int col);

        bool propagateFixedValues();
    };

}  // namespace qm::latin_square
