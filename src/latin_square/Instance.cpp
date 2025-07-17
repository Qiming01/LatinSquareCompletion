#include "latin_square/Instance.h"
#include <iostream>

namespace qm::latin_square {

    std::istream &operator>>(std::istream &is, Instance &instance) {
        int n = 0;
        is >> n;
        instance.n_ = n;
        instance.fixed_numbers_.clear();
        for (Assignment a; is >> a.row >> a.col >> a.num;) {
            instance.fixed_numbers_.push_back(a);
        }
        return is;
    }

    std::ostream &operator<<(std::ostream &os, const Instance &instance) {
        os << instance.n_ << '\n';
        for (const auto &a: instance.fixed_numbers_) {
            os << a.row << ' ' << a.col << ' ' << a.num << '\n';
        }
        return os;
    }

}
