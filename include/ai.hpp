#pragma once

#include "gomoku.hpp"
#include <vector>

// Move generation
std::vector<Move> generate_candidates(const GameState &s);
void order_moves(std::vector<Move> &moves, GameState &s, int tt_best);

// Minimax
int minimax(GameState &s, int depth, int alpha, int beta,
            bool maximizing, Cell ai_color, int &node_count,
            double start_time, bool &timed_out);

// Quick eval for move ordering / heatmap
int quick_eval_move(const GameState &s, int pos, Cell color);
int count_capturable_pairs(const int8_t *board, Cell color);
