#pragma once
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <mutex>
#include "board_2048.hpp"
namespace core {
    struct solver {
        using eval_t = uint64_t;
        static constexpr int CACHE_DEPTH = 2;
        static constexpr int MAX_DEPTH = 10;
        static constexpr int USUAL_CACHE = 1 << 16;
        static constexpr eval_t MIN_EVAL = 0;
        static constexpr eval_t MAX_EVAL = 16ULL << 41;
        static constexpr eval_t MULT = 9e18 / (MAX_EVAL * 10 * 4 * 30 * 4 * 16);
        static constexpr int MAX_CACHE = 1 << 20;

        explicit solver(int depth = 2)
            : depth(depth){
            init_cache();
        }

        int get_best_move(const board_2048& board_) {
            return pick_move(board_);
        }

        void set_depth(int depth) {
            this->depth = depth;
        }
    private:
        int depth;
        using cache_t = std::unordered_map<board_2048, eval_t>;
        cache_t cache;
        board_2048 deletion_queue[MAX_CACHE];
        int q[4] = { 0, 0, 0, 0 };
        int q_end = 0;

        void init_cache() {
            cache.reserve(USUAL_CACHE);
        }

        void add_to_cache(const board_2048& board, const eval_t score, const int move, const int depth) {
            cache[board] = (((score << 2) | move) << 4) | depth;

            // relies on MAX_CACHE being a power of 2
            deletion_queue[q_end & (MAX_CACHE - 1)] = board;
            ++q_end;
            if (q[0] + MAX_CACHE == q_end) {
                cache.erase(deletion_queue[q[0]++]);
                if (q[0] >= MAX_CACHE) {
                    q[0] -= MAX_CACHE;
                    q[1] -= MAX_CACHE;
                    q[2] -= MAX_CACHE;
                    q[3] -= MAX_CACHE;
                    q_end -= MAX_CACHE;
                }
            }
        }

        void update_cache_pointers() {
            while (q[0] < q[1]) {  // delete everything in range q_0 ... q_1
                cache.erase(deletion_queue[q[0]++]);
                if (q[0] >= MAX_CACHE) {
                    q[0] -= MAX_CACHE;
                    q[1] -= MAX_CACHE;
                    q[2] -= MAX_CACHE;
                    q[3] -= MAX_CACHE;
                    q_end -= MAX_CACHE;
                }
            }
            q[1] = std::max(q[0], q[2]);
            q[2] = std::max(q[0], q[3]);  // possible that stuff was deleted while searching because cache got too big
            q[3] = q_end;
        }

        int pick_move(const board_2048& board) {
            const int depth_to_use = depth <= 0 ? pick_depth(board) - depth : depth;

            const int move = expectimax(board, depth_to_use, 0) & 3;
            update_cache_pointers();
            return move;
        }

    private:
        eval_t expectimax(const board_2048& board, const int cur_depth, const int fours) {
            if (board.is_over()) {
                const eval_t score = MULT * evaluate_board(board);
                return (score - (score >> 2)) << 2;  // subtract score / 4 as penalty for dying, then pack
            }
            if (cur_depth == 0 || fours >= 4) {  // selecting 4 fours has a 0.01% chance, which is negligible
                return (MULT * evaluate_board(board)) << 2; 
            }

            if (cur_depth >= CACHE_DEPTH) {
                const auto it = cache.find(board);
#ifdef REQUIRE_DETERMINISTIC
                if (it != cache.end() && (it->second & 0xF) == cur_depth) return it->second >> 4;
#else
                if (it != cache.end() && (it->second & 0xF) >= cur_depth) return it->second >> 4;
#endif
            }

            eval_t best_score = MIN_EVAL;
            int best_move = -1;
            for (int i = direction::left; i < 4; ++i) {
                eval_t expected_score = 0;
                board_2048 new_board = board;
                new_board.move(i);
                if (board == new_board) {
                    continue;
                }
                else {
                    int cnt_empty = 0;
                    for (auto& tile : new_board.brd) {
                        if (!tile) {
                            tile = 2;
                            expected_score += 9 * (expectimax(new_board, cur_depth - 1, fours) >> 2);
                            tile = 4;
                            expected_score += 1 * (expectimax(new_board, cur_depth - 1, fours + 1) >> 2);
                            tile = 0;
                            ++cnt_empty;
                        }
                    }
                    expected_score /= cnt_empty * 10;  // convert to actual expected score * MULT
                }

                if (best_score <= expected_score) {
                    best_score = expected_score;
                    best_move = i;
                }
            }

            if (cur_depth >= CACHE_DEPTH) {
                add_to_cache(board, best_score, best_move, cur_depth);
            }

            return (best_score << 2) | best_move;  // pack both score and move
        }

        int pick_depth(const board_2048& board) {
            const int tile_ct = board.count_tiles();
            const int score = board.count_distinct_tiles() + (tile_ct <= 6 ? 0 : (tile_ct - 6) >> 1);
            return 2 + (score >= 8) + (score >= 11) + (score >= 14) + (score >= 15) + (score >= 17) + (score >= 19);
        }

        eval_t evaluate_board(const board_2048& board) {
            int size = board.size();
            auto calculate_corner_value = [&](int start_x, int start_y, int dx, int dy) {
                eval_t value = 0;
                int init_weight = 20, weight = 20;
                for (int i = 0; i < size; ++i) {
                    weight = init_weight;
                    for (int j = 0; j < size - i; ++j) {
                        value += weight * board.get_tile(start_x + i * dx, start_y + j * dy);
                        weight = std::max(1, weight / 2);
                    }
                    init_weight /= 2;
                }
                return value;
            };

            const eval_t lower_left = calculate_corner_value(0, size - 1, 1, -1);
            const eval_t upper_left = calculate_corner_value(size - 1, size - 1, -1, -1);
            const eval_t lower_right = calculate_corner_value(0, 0, 1, 1);
            const eval_t upper_right = calculate_corner_value(size - 1, 0, -1, 1);

            return std::max({ lower_left, upper_left, lower_right, upper_right });
        }
    };
}
