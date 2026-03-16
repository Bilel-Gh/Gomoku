#include "gomoku.hpp"
#include <cstring>

void init_game(GameState &s) {
	memset(s.board, EMPTY, sizeof(s.board));
	s.captures[0] = 0;
	s.captures[1] = 0;
	s.current = STONE_BLACK;
	s.move_count = 0;
	s.last_move = -1;
	s.zhash = 0;
	s.game_over = false;
	s.winner = EMPTY;
	s.pending_five_pos = -1;
	s.pending_five_color = EMPTY;
}

// Check captures in all 8 directions, return count of pairs captured
int check_captures(const GameState &s, int pos, Cell color) {
	int pairs = 0;
	Cell opp = opponent(color);
	int r = row_of(pos), c = col_of(pos);

	for (int d = 0; d < 4; d++) {
		for (int sign = -1; sign <= 1; sign += 2) {
			int dr = DIR[d][0] * sign;
			int dc = DIR[d][1] * sign;
			int r1 = r + dr, c1 = c + dc;
			int r2 = r + 2*dr, c2 = c + 2*dc;
			int r3 = r + 3*dr, c3 = c + 3*dc;
			if (!in_bounds(r1, c1) || !in_bounds(r2, c2) || !in_bounds(r3, c3))
				continue;
			if (s.board[idx(r1, c1)] == opp &&
				s.board[idx(r2, c2)] == opp &&
				s.board[idx(r3, c3)] == color) {
				pairs++;
			}
		}
	}
	return pairs;
}

// Apply captures and record them in undo struct
void apply_captures(GameState &s, int pos, Cell color, MoveUndo &undo) {
	Cell opp = opponent(color);
	int r = row_of(pos), c = col_of(pos);
	undo.num_captured = 0;

	for (int d = 0; d < 4; d++) {
		for (int sign = -1; sign <= 1; sign += 2) {
			int dr = DIR[d][0] * sign;
			int dc = DIR[d][1] * sign;
			int r1 = r + dr, c1 = c + dc;
			int r2 = r + 2*dr, c2 = c + 2*dc;
			int r3 = r + 3*dr, c3 = c + 3*dc;
			if (!in_bounds(r1, c1) || !in_bounds(r2, c2) || !in_bounds(r3, c3))
				continue;
			int i1 = idx(r1, c1), i2 = idx(r2, c2), i3 = idx(r3, c3);
			if (s.board[i1] == opp && s.board[i2] == opp && s.board[i3] == color) {
				// Remove captured pair
				s.zhash ^= zobrist_table[i1][(int)s.board[i1]];
				s.zhash ^= zobrist_table[i2][(int)s.board[i2]];
				s.board[i1] = EMPTY;
				s.board[i2] = EMPTY;
				undo.captured[undo.num_captured++] = i1;
				undo.captured[undo.num_captured++] = i2;
				s.captures[color - 1]++;
			}
		}
	}
}

// Check if color has 5+ in a row through position pos
bool check_five(const GameState &s, int pos, Cell color) {
	if (s.board[pos] != color) return false;
	int r = row_of(pos), c = col_of(pos);

	for (int d = 0; d < 4; d++) {
		int count = 1;
		// positive direction
		for (int step = 1; step < BSIZE; step++) {
			int nr = r + step * DIR[d][0];
			int nc = c + step * DIR[d][1];
			if (!in_bounds(nr, nc) || s.board[idx(nr, nc)] != color) break;
			count++;
		}
		// negative direction
		for (int step = 1; step < BSIZE; step++) {
			int nr = r - step * DIR[d][0];
			int nc = c - step * DIR[d][1];
			if (!in_bounds(nr, nc) || s.board[idx(nr, nc)] != color) break;
			count++;
		}
		if (count >= 5) return true;
	}
	return false;
}

// Get all positions in the five-in-a-row line
void get_five_positions(const int8_t *board, int pos, Cell color, int *positions, int &count) {
	int r = row_of(pos), c = col_of(pos);
	count = 0;

	for (int d = 0; d < 4; d++) {
		int tmp[BSIZE];
		int n = 0;
		// negative direction
		for (int step = BSIZE - 1; step >= 1; step--) {
			int nr = r - step * DIR[d][0];
			int nc = c - step * DIR[d][1];
			if (!in_bounds(nr, nc) || board[idx(nr, nc)] != color) continue;
			// only add if contiguous from the found start
		}
		// Better approach: walk from pos in both directions
		n = 0;
		tmp[n++] = pos;
		for (int step = 1; step < BSIZE; step++) {
			int nr = r + step * DIR[d][0];
			int nc = c + step * DIR[d][1];
			if (!in_bounds(nr, nc) || board[idx(nr, nc)] != color) break;
			tmp[n++] = idx(nr, nc);
		}
		for (int step = 1; step < BSIZE; step++) {
			int nr = r - step * DIR[d][0];
			int nc = c - step * DIR[d][1];
			if (!in_bounds(nr, nc) || board[idx(nr, nc)] != color) break;
			tmp[n++] = idx(nr, nc);
		}
		if (n >= 5) {
			for (int i = 0; i < n; i++)
				positions[count++] = tmp[i];
			return;
		}
	}
}

// Full win check: five-in-a-row (with endgame capture rule) or capture win
bool check_win(GameState &s, int pos, Cell color) {
	// Capture win
	if (s.captures[color - 1] >= 5) {
		s.game_over = true;
		s.winner = color;
		return true;
	}

	// Five-in-a-row
	if (check_five(s, pos, color)) {
		// Endgame capture rule: check if opponent can break it
		if (can_opponent_break_five(s, pos, color)) {
			s.pending_five_pos = pos;
			s.pending_five_color = color;
			return false; // opponent gets one chance to break it
		}
		s.game_over = true;
		s.winner = color;
		return true;
	}

	// Check draw (board full)
	if (s.move_count >= BCELLS) {
		s.game_over = true;
		s.winner = EMPTY;
		return true;
	}

	return false;
}

// Place stone: validate, place, capture, check win
bool place_stone(GameState &s, int pos) {
	if (pos < 0 || pos >= BCELLS || s.board[pos] != EMPTY || s.game_over)
		return false;
	if (!is_legal_move(s, pos))
		return false;

	Cell color = s.current;
	s.board[pos] = color;
	s.zhash ^= zobrist_table[pos][(int)color];
	s.move_count++;
	s.last_move = pos;

	// Apply captures
	MoveUndo dummy;
	apply_captures(s, pos, color, dummy);

	// Check if opponent had a pending five that we failed to break
	if (s.pending_five_pos >= 0 && s.pending_five_color != color) {
		// Check if the five is still intact after our captures
		if (check_five(s, s.pending_five_pos, s.pending_five_color)) {
			// Opponent's five survives — they win (unless we won by captures)
			if (s.captures[color - 1] < 5) {
				s.game_over = true;
				s.winner = s.pending_five_color;
				s.pending_five_pos = -1;
				s.pending_five_color = EMPTY;
				return true;
			}
		}
		// Five was broken or we won by captures — clear pending
		s.pending_five_pos = -1;
		s.pending_five_color = EMPTY;
	}

	// Check win for current move
	check_win(s, pos, color);

	// Switch turn
	if (!s.game_over)
		s.current = opponent(color);

	return true;
}

Snapshot make_snapshot(const GameState &s) {
	Snapshot snap;
	memcpy(snap.board, s.board, sizeof(s.board));
	snap.captures[0] = s.captures[0];
	snap.captures[1] = s.captures[1];
	snap.current = s.current;
	snap.move_count = s.move_count;
	snap.last_move = s.last_move;
	snap.zhash = s.zhash;
	snap.game_over = s.game_over;
	snap.winner = s.winner;
	snap.pending_five_pos = s.pending_five_pos;
	snap.pending_five_color = s.pending_five_color;
	return snap;
}

void restore_snapshot(GameState &s, const Snapshot &snap) {
	memcpy(s.board, snap.board, sizeof(s.board));
	s.captures[0] = snap.captures[0];
	s.captures[1] = snap.captures[1];
	s.current = snap.current;
	s.move_count = snap.move_count;
	s.last_move = snap.last_move;
	s.zhash = snap.zhash;
	s.game_over = snap.game_over;
	s.winner = snap.winner;
	s.pending_five_pos = snap.pending_five_pos;
	s.pending_five_color = snap.pending_five_color;
}
