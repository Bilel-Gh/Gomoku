#include "gomoku.hpp"
#include "ai.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>

static double now_seconds() {
	static auto start = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(now - start).count();
}

// Fast apply move for AI search
void ai_apply_move(GameState &s, int pos, MoveUndo &undo) {
	undo.pos = pos;
	undo.prev_hash = s.zhash;
	undo.prev_current = s.current;
	undo.prev_move_count = s.move_count;
	undo.prev_last_move = s.last_move;
	undo.num_captured = 0;

	Cell color = s.current;
	s.board[pos] = color;
	s.zhash ^= zobrist_table[pos][(int)color];
	s.move_count++;
	s.last_move = pos;

	// Apply captures
	apply_captures(s, pos, color, undo);

	s.current = opponent(color);
}

// Fast undo move for AI search
void ai_undo_move(GameState &s, MoveUndo &undo) {
	s.current = undo.prev_current;
	s.move_count = undo.prev_move_count;
	s.last_move = undo.prev_last_move;

	Cell color = undo.prev_current;
	Cell opp = opponent(color);

	// Restore captured stones
	for (int i = 0; i < undo.num_captured; i++)
		s.board[undo.captured[i]] = opp;
	s.captures[color - 1] -= undo.num_captured / 2;

	// Remove the placed stone
	s.board[undo.pos] = EMPTY;
	s.zhash = undo.prev_hash;
}

// Generate candidate moves (within distance 2 of existing stones)
std::vector<Move> generate_candidates(const GameState &s) {
	std::vector<Move> moves;

	if (s.move_count == 0) {
		moves.push_back({idx(BSIZE / 2, BSIZE / 2), 0});
		return moves;
	}

	bool candidate[BCELLS] = {};
	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] == EMPTY) continue;
		int r = row_of(i), c = col_of(i);
		for (int dr = -CANDIDATE_DISTANCE; dr <= CANDIDATE_DISTANCE; dr++) {
			for (int dc = -CANDIDATE_DISTANCE; dc <= CANDIDATE_DISTANCE; dc++) {
				int nr = r + dr, nc = c + dc;
				if (in_bounds(nr, nc) && s.board[idx(nr, nc)] == EMPTY)
					candidate[idx(nr, nc)] = true;
			}
		}
	}

	for (int i = 0; i < BCELLS; i++) {
		if (!candidate[i]) continue;
		if (!is_legal_move(s, i)) continue;
		moves.push_back({i, 0});
	}

	return moves;
}

// Score moves for ordering
void order_moves(std::vector<Move> &moves, GameState &s, int tt_best) {
	Cell color = s.current;
	Cell opp = opponent(color);

	for (auto &m : moves) {
		if (m.pos == tt_best) {
			m.score = ORDER_TT_MOVE;
			continue;
		}

		// Check if this move wins (five in a row)
		s.board[m.pos] = color;
		if (check_five(s, m.pos, color)) {
			m.score = ORDER_WIN;
			s.board[m.pos] = EMPTY;
			continue;
		}
		s.board[m.pos] = EMPTY;

		// Check captures
		int caps = check_captures(s, m.pos, color);
		if (caps > 0) {
			m.score = ORDER_CAPTURE * caps;
			if (s.captures[color - 1] + caps >= 5)
				m.score = ORDER_WIN; // capture win
			continue;
		}

		// Quick heuristic: own score + blocking opponent
		int own = quick_eval_move(s, m.pos, color);
		int block = quick_eval_move(s, m.pos, opp);
		m.score = own + block / 2;
	}

	std::sort(moves.begin(), moves.end(),
		[](const Move &a, const Move &b) { return a.score > b.score; });
}

// Minimax with alpha-beta pruning
int minimax(GameState &s, int depth, int alpha, int beta,
            bool maximizing, Cell ai_color, int &node_count,
            double start_time, bool &timed_out) {
	node_count++;

	// Time check
	if ((node_count & (TIME_CHECK_INTERVAL - 1)) == 0) {
		if (now_seconds() - start_time > TIME_LIMIT) {
			timed_out = true;
			return 0;
		}
	}

	// TT lookup
	TTEntry *tt_entry = tt_lookup(s.zhash);
	int tt_best = -1;
	if (tt_entry && tt_entry->depth >= depth) {
		if (tt_entry->flag == TT_EXACT)
			return tt_entry->score;
		if (tt_entry->flag == TT_ALPHA && tt_entry->score <= alpha)
			return alpha;
		if (tt_entry->flag == TT_BETA && tt_entry->score >= beta)
			return beta;
	}
	if (tt_entry)
		tt_best = tt_entry->best_move;

	// Terminal: check for wins
	// If last move caused a win, evaluate it
	if (s.last_move >= 0) {
		Cell last_color = opponent(s.current); // color that just moved
		if (s.captures[last_color - 1] >= 5)
			return (last_color == ai_color) ? WIN_SCORE + depth : -(WIN_SCORE + depth);
		if (check_five(s, s.last_move, last_color))
			return (last_color == ai_color) ? WIN_SCORE + depth : -(WIN_SCORE + depth);
	}

	// Leaf node
	if (depth == 0)
		return evaluate(s, ai_color);

	// Generate and order moves
	std::vector<Move> moves = generate_candidates(s);
	if (moves.empty())
		return evaluate(s, ai_color);

	order_moves(moves, s, tt_best);

	int original_alpha = alpha;
	int best_score = maximizing ? -200000000 : 200000000;
	int best_move = moves[0].pos;

	for (auto &m : moves) {
		MoveUndo undo;
		ai_apply_move(s, m.pos, undo);
		int score = minimax(s, depth - 1, alpha, beta, !maximizing,
		                    ai_color, node_count, start_time, timed_out);
		ai_undo_move(s, undo);

		if (timed_out) return 0;

		if (maximizing) {
			if (score > best_score) {
				best_score = score;
				best_move = m.pos;
			}
			alpha = std::max(alpha, score);
		} else {
			if (score < best_score) {
				best_score = score;
				best_move = m.pos;
			}
			beta = std::min(beta, score);
		}

		if (beta <= alpha)
			break;
	}

	// TT store
	TTFlag flag = TT_EXACT;
	if (best_score <= original_alpha) flag = TT_ALPHA;
	else if (best_score >= beta) flag = TT_BETA;
	tt_store(s.zhash, depth, best_score, flag, best_move);

	return best_score;
}

// Main AI entry point with iterative deepening
int ai_get_move(GameState &s, Cell ai_color, double &elapsed_out) {
	double start = now_seconds();
	int best_move = -1;
	bool is_max = (s.current == ai_color);

	tt_generation++;

	// First move: play center
	if (s.move_count == 0) {
		elapsed_out = now_seconds() - start;
		return idx(BSIZE / 2, BSIZE / 2);
	}

	// Iterative deepening
	for (int depth = 2; depth <= 20; depth += 2) {
		int node_count = 0;
		bool timed_out = false;

		// Root search
		std::vector<Move> moves = generate_candidates(s);
		if (moves.empty()) break;
		if (moves.size() == 1) {
			best_move = moves[0].pos;
			break;
		}

		int tt_best = best_move;
		order_moves(moves, s, tt_best);

		int best_score = is_max ? -200000000 : 200000000;
		int alpha = -200000000;
		int beta = 200000000;
		int move_for_depth = moves[0].pos;

		for (auto &m : moves) {
			MoveUndo undo;
			ai_apply_move(s, m.pos, undo);
			int score = minimax(s, depth - 1, alpha, beta, !is_max,
			                    ai_color, node_count, start, timed_out);
			ai_undo_move(s, undo);

			if (timed_out) break;

			if (is_max) {
				if (score > best_score) {
					best_score = score;
					move_for_depth = m.pos;
				}
				alpha = std::max(alpha, score);
			} else {
				if (score < best_score) {
					best_score = score;
					move_for_depth = m.pos;
				}
				beta = std::min(beta, score);
			}
		}

		if (timed_out)
			break;

		best_move = move_for_depth;

		// Found forced win, stop early
		if (best_score >= WIN_SCORE || best_score <= -WIN_SCORE)
			break;

		// Soft time limit: don't start next iteration if running low
		if (now_seconds() - start > TIME_SOFT_LIMIT)
			break;
	}

	elapsed_out = now_seconds() - start;
	return best_move;
}

// Quick suggestion (lower depth, for hint mode)
int ai_suggest_move(GameState &s, Cell color) {
	double dummy;
	// Use a copy so we don't mess with state
	GameState tmp;
	memcpy(&tmp, &s, sizeof(GameState));
	return ai_get_move(tmp, color, dummy);
}
