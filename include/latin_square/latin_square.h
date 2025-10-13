//
// Created by qiming on 25-7-17.
//

#pragma once

#include <memory>
#include "latin_square/instance.h"
#include "color_domain.h"

namespace qm::latin_square {
struct Solution {
    int row_conflict{};
    int column_conflict{};
    int total_conflict{};
    std::vector<std::vector<int>> solution;
};

class Latin_square {
    ColorDomain color_domain;
    Solution solution;
};
}
