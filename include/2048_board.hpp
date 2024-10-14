#pragma once
#include "coord.hpp"
#include <algorithm>
#include <random>
#include <vector>

namespace core {

class board_2048 {
public:
    board_2048() = default;
    board_2048(int size)
        : brd_size(size)
        , gen(std::random_device {}())
    {
        brd.resize(size * size, 0);
        add_random_tile();
        add_random_tile();
    }

    void move(int dir);

    int get_tile(int x, int y) const { return brd[x * brd_size + y]; }

private:
    std::default_random_engine gen;
    int brd_size = 4;
    std::vector<int> brd;

    void add_random_tile();
    void slide_and_merge_row(std::vector<int>& row);
    void rotate_board_r();
};

void board_2048::add_random_tile()
{
    std::uniform_int_distribution<int> dist(0, brd_size * brd_size - 1);
    int index;
    do {
        index = dist(gen);
    } while (brd[index] != 0);
    brd[index] = (dist(gen) % 2 + 1) * 2;
}

void board_2048::slide_and_merge_row(std::vector<int>& row)
{
    // Slide non-zero elements to the left
    std::vector<int> new_row(brd_size, 0);
    int index = 0;
    for (int i = 0; i < brd_size; ++i) {
        if (row[i] != 0) {
            new_row[index++] = row[i];
        }
    }
    // Merge adjacent equal elements
    for (int i = 0; i < brd_size - 1; ++i) {
        if (new_row[i] != 0 && new_row[i] == new_row[i + 1]) {
            new_row[i] *= 2;
            new_row[i + 1] = 0;
        }
    }
    // Slide again after merging
    std::vector<int> final_row(brd_size, 0);
    index = 0;
    for (int i = 0; i < brd_size; ++i) {
        if (new_row[i] != 0) {
            final_row[index++] = new_row[i];
        }
    }
    row = std::move(final_row);
}

void board_2048::rotate_board_r()
{
    std::vector<int> new_brd(brd_size * brd_size);
    for (int i = 0; i < brd_size; ++i) {
        for (int j = 0; j < brd_size; ++j) {
            new_brd[j * brd_size + (brd_size - 1 - i)] = brd[i * brd_size + j];
        }
    }
    brd = std::move(new_brd);
}

void board_2048::move(int dir)
{
    // Rotate the board to simplify the move logic
    for (int i = 0; i < dir; ++i) {
        rotate_board_r();
    }

    // Move left
    for (int i = 0; i < brd_size; ++i) {
        std::vector<int> row(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size);
        slide_and_merge_row(row);
        std::copy(row.begin(), row.end(), brd.begin() + i * brd_size);
    }

    // Rotate the board back to the original orientation
    for (int i = 0; i < (4 - dir) % 4; ++i) {
        rotate_board_r();
    }

    // Add a new random tile
    add_random_tile();
}
}