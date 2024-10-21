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
    struct solver;
    inline std::default_random_engine gen(std::random_device{}());
class board_2048 {
public:
    friend class tui::BoardBase;
    friend struct solver;
    using iter_type = std::vector<int>::iterator;
    board_2048(int size = 4)
        : brd_size(size)
    {
        brd.resize(size * size, 0);
        add_random_tile();
        if (size * size >= 2) {
            add_random_tile();
        }
    }

    void move(int dir);

    void move_record(int dir);

    bool valid_move(int dir) const;

    void add_random_tile();

    int get_tile(int x, int y) const { return brd[x * brd_size + y]; }

    void set_tile(int x, int y, int val) { brd[x * brd_size + y] = val; }

    int count_tiles() const {
        int cnt = 0;
        for (auto& n : brd) {
            if (n) {
                ++cnt;
            }
        }
        return cnt;
    }

    int count_empty_tiles() const {
        int cnt = 0;
        for (auto& n : brd) {
            if (!n) {
                ++cnt;
            }
        }
        return cnt;
    }

    int count_distinct_tiles() const {
        uint64_t mask = 0;
        for (auto& tile : brd) {
            mask |= tile;
        }
        return std::popcount(mask);
    }

    int size() const {
        return brd_size;
    }

    bool is_over() const { 
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

    uint64_t hash() const {
        uint64_t hash_value = 0;
        for (int i = 0; i < brd.size(); ++i) {
            hash_value = hash_value * 31 + brd[i];
        }
        return hash_value;
    }


    bool operator==(const board_2048& brd) const {
        return this->brd == brd.brd;
    }

    uint64_t get_score() const { return score;}
private:
    int brd_size = 4;
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

    void rotate_to_left_from(int);

    void rotate_from_left_to(int);
};

void board_2048::add_random_tile()
{
    std::uniform_int_distribution<int> dist(0, brd_size * brd_size - 1);
    int index;
    do {
        index = dist(gen);
    } while (brd[index] != 0);
    brd[index] = dist(gen) % 10 == 0 ? 4 : 2;
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

inline void board_2048::rotate_to_left_from(int dir)
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
    rotate_to_left_from(dir);

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

    rotate_to_left_from(dir);

    for (int i = 0; i < brd_size; ++i) {
        init_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        slide_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        merge_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
        slide_row_record(brd.begin() + i * brd_size, brd.begin() + (i + 1) * brd_size, i);
    }

    rotate_from_left_to(dir);
}

inline bool board_2048::valid_move(int dir) const 
{
    board_2048 test_brd = *this;
    std::vector<int> original_brd = test_brd.brd;

    test_brd.rotate_to_left_from(dir);

    // Move left
    for (int i = 0; i < brd_size; ++i) {
        test_brd.slide_and_merge_row(test_brd.brd.begin() + i * brd_size, test_brd.brd.begin() + (i + 1) * test_brd.brd_size);
    }

    test_brd.rotate_from_left_to(dir);

    bool ret = false;
    if (test_brd.brd != original_brd) {
        ret = true;
    }
    else {
        ret = false;
    }
    return ret;
}
}
template<>
struct std::hash<core::board_2048> {
    uint64_t operator()(const core::board_2048& brd) const {
        return brd.hash();
    }
};