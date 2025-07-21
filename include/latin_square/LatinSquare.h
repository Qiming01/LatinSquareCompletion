//
// Created by qiming on 25-7-17.
//

#pragma once

#include <memory>
#include "latin_square/Instance.h"
#include "ColorDomain.h"

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
