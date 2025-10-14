//
// Created by 祁明 on 2025/10/14.
//

#ifndef LATINSQUARECOMPLETION_MOVE_H
#define LATINSQUARECOMPLETION_MOVE_H
namespace qm::latin_square {
// 邻域动作，交换第row_id行第col1列和第col2列的元素
struct Move {
    int row_id;
    int col1;
    int col2;
};
}// namespace qm::latin_square
#endif// LATINSQUARECOMPLETION_MOVE_H
