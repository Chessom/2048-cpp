#pragma once
#include "coord.hpp"
#include <algorithm>
#include <random>
#include <vector>
#include <ranges>
namespace tui {
    class BoardBase;
}
namespace core {

class board_2048 {
public:
    friend class tui::BoardBase;
    using iter_type = std::vector<int>::iterator;
    board_2048(int size = 4)
        : brd_size(size)
        , gen(std::random_device {}())
    {
        brd.resize(size * size, 0);
        add_random_tile();
        add_random_tile();
    }

    void move(int dir);

    void move_record(int dir);

    bool valid_move(int dir);

    void add_random_tile();

    int get_tile(int x, int y) const { return brd[x * brd_size + y]; }

    int size() const {
        return brd_size;
    }

    bool is_over() { 
        if (
            valid_move(direction::left)||
            valid_move(direction::down)||
            valid_move(direction::right)||
            valid_move(direction::up)
            ) {
            return false;
        }
        else {
            return true;
        }
    }

    uint64_t scroe() const { return score;}
private:
    std::default_random_engine gen;
    int brd_size = 4;
    bool over = false;
    uint64_t score = 0;
    std::vector<int> brd;
    std::vector<std::pair<int, int>> records;

    int pos2n(int x, int y)const { return x * brd_size + y; };

    void slide_row(std::vector<int>& row) { slide_row(row.begin(), row.end()); };

    void slide_row(iter_type begin, iter_type end);

    void slide_row_record(iter_type begin, iter_type end, int row);

    void init_row_record(iter_type begin, iter_type end, int row);

    void merge_row(std::vector<int>& row) { merge_row(row.begin(), row.end()); };

    void merge_row(iter_type begin, iter_type end);

    void merge_row_record(iter_type begin, iter_type end, int row);

    void slide_and_merge_row(std::vector<int>& row) { slide_and_merge_row(row.begin(), row.end()); };

    void slide_and_merge_row(iter_type begin, iter_type end);

    void rotate_board_r();

    void rotate_board_l();

    void rotate_board_180();

    void rotate_to_left(int);

    void rotate_from_left_to(int);
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

inline void board_2048::slide_row(iter_type begin, iter_type end)
{
    // Slide non-zero elements to the left
    iter_type index = begin;
    for (iter_type it = begin; it != end; ++it) {
        if (*it != 0) {
            if (index == it) {
                index++;
            }
            else {
                *index++ = *it;
                *it = 0;
            }
        }
    }
}

inline void board_2048::slide_row_record(iter_type begin, iter_type end, int row)
{
    // Slide non-zero elements to the left and record where the elements go
    iter_type index = begin;//the last zero element
    for (iter_type it = begin; it != end; ++it) {
        if (*it != 0) {
            if (index == it) {
                index++;
            }
            else {
                //pair: first: value | second: where to go
                for (auto j = it; j < end; ++j) {
                    auto& p = records[pos2n(row, (j - begin))];
                    if (p.second == pos2n(row, it - begin)) {
                        p.second = pos2n(row, index - begin);
                    }
                }
                *index++ = *it;
                *it = 0;
            }
        }
    }
}

inline void board_2048::init_row_record(iter_type begin, iter_type end, int row)
{
    iter_type index = begin;//the last zero element
    for (iter_type it = begin; it != end; ++it) {
		if (*it != 0) {
            records[pos2n(row, (it - begin))] = { *it, pos2n(row, (it - begin)) };
        }
    }
}

inline void board_2048::merge_row(iter_type begin, iter_type end)
{
    // Merge adjacent equal elements
    for (iter_type it = begin; it != end - 1; ++it) {
        if (*it != 0 && *it == *(it + 1)) {
            *it += *it;
            *(it + 1) = 0;
            score += *it;
        }
    }
}

inline void board_2048::merge_row_record(iter_type begin, iter_type end, int row)
{
    // Merge adjacent equal elements and update target pos;
    for (iter_type it = begin; it != end - 1; ++it) {
        if (*it != 0 && *it == *(it + 1)) {
            for (auto j = it + 1; j < end; ++j) {
                if (auto& p = records[pos2n(row, (j - begin))];
                    p.first != 0 &&
                    p.second == pos2n(row, it + 1 - begin)) 
                {
                    p.second -= 1;
                }
            }
            *it += *it;
            *(it + 1) = 0;
            score += *it;
        }
    }
}

void board_2048::slide_and_merge_row(iter_type begin, iter_type end)
{
    slide_row(begin, end);
    merge_row(begin, end);
    slide_row(begin, end);
}

void board_2048::rotate_board_r()
{
    std::vector<int> new_brd(brd_size * brd_size);
    for (int i = 0; i < brd_size; ++i) {
        for (int j = 0; j < brd_size; ++j) {
            new_brd[j * brd_size + (brd_size - 1 -i)] = brd[i * brd_size + j];
        }
    }
    brd = std::move(new_brd);
}

void board_2048::rotate_board_l()
{
    std::vector<int> new_brd(brd_size * brd_size);
    for (int i = 0; i < brd_size; ++i) {
        for (int j = 0; j < brd_size; ++j) {
            new_brd[(brd_size - 1 - j) * brd_size + i] = brd[i * brd_size + j];
        }
    }
    brd = std::move(new_brd);
}

void board_2048::rotate_board_180() {
    std::vector<int> new_brd(brd_size * brd_size);
    for (int i = 0; i < brd_size; ++i) {
        for (int j = 0; j < brd_size; ++j) {
            new_brd[(brd_size - 1 - i) * brd_size + brd_size - 1 - j] = brd[i * brd_size + j];
        }
    }
    brd = std::move(new_brd);
}

inline void board_2048::rotate_to_left(int dir)
{
    // Rotate the board to simplify the move logic
    switch (dir)
    {
    case direction::left:
        break;
    case direction::down:
        rotate_board_r();
        break;
    case direction::right:
        rotate_board_180();
        break;
    case direction::up:
        rotate_board_l();
        break;
    default:
        break;
    }
}

inline void board_2048::rotate_from_left_to(int dir)
{
    // Rotate the board back to the original orientation
    switch (dir)
    {
    case direction::left:
        break;
    case direction::down:
        rotate_board_l();
        break;
    case direction::right:
        rotate_board_180();
        break;
    case direction::up:
        rotate_board_r();
        break;
    default:
        break;
    }
}

void board_2048::move(int dir)
{
    rotate_to_left(dir);

    // Move left
    for (int i = 0; i < brd_size; ++i) {
        slide_and_merge_row(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size);
    }

    rotate_from_left_to(dir);
}

inline void board_2048::move_record(int dir)
{
    records.clear();
    records.resize(brd_size * brd_size);

    rotate_to_left(dir);

    for (int i = 0; i < brd_size; ++i) {
        init_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        slide_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        merge_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        slide_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
    }

    rotate_from_left_to(dir);
}

inline bool board_2048::valid_move(int dir)
{
    std::vector<int> original_brd = brd;

    rotate_to_left(dir);

    // Move left
    for (int i = 0; i < brd_size; ++i) {
        slide_and_merge_row(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size);
    }

    rotate_from_left_to(dir);

    // Add a new random tile
    bool ret = false;
    if (brd != original_brd) {
        ret = true;
    }
    else {
        ret = false;
    }
    brd = original_brd;
    return ret;
}
}