#pragma once

#include <vector>
#include <iostream>

namespace qm::latin_square {

    struct Assignment {
        int row{0};
        int col{0};
        int num{0};

        Assignment() = default;

        Assignment(int row, int col, int num) : row(row), col(col), num(num) {}
    };

    class Instance {
    public:
        [[nodiscard]] int size() const { return n_; }

        [[nodiscard]] const std::vector<Assignment> &get_fixed() const { return fixed_numbers_; }

        [[nodiscard]] const std::vector<Assignment> &fixed() const { return fixed_numbers_; }

        friend std::istream &operator>>(std::istream &is, Instance &instance);

        friend std::ostream &operator<<(std::ostream &os, const Instance &instance);

    private:
        int n_{0};
        std::vector<Assignment> fixed_numbers_;
    };

}
