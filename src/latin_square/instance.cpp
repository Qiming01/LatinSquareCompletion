#include "latin_square/instance.h"
#include <iostream>

namespace qm::latin_square {

std::istream &operator>>(std::istream &is, Instance &instance) {
    int n = 0;
    is >> n;
    instance.n_ = n;

    instance.fixed_.clear();
    instance.fixed_.reserve(n * n);

    int r, c, v;
    while (is >> r >> c >> v) { instance.fixed_.emplace_back(r, c, v); }

    return is;
}

std::ostream &operator<<(std::ostream &os, const Instance &instance) {
    os << instance.n_ << '\n';
    for (const auto &a: instance.fixed_) { os << a.row << ' ' << a.col << ' ' << a.num << '\n'; }
    return os;
}

}
