#include "gomoku.hpp"

// Check if placing color at pos creates a free-three along direction (dr, dc)
// A free-three: exactly 3 same-color stones (including the one being placed)
// with at most 1 gap, and both ends open (EMPTY + in bounds)
bool is_free_three(const int8_t *board, int pos, Cell color, int dr, int dc) {
	int r = row_of(pos), c = col_of(pos);

	// We need to check all windows of size 4-5 along this direction
	// that include pos, and see if placing at pos creates a free-three pattern.
	//
	// Free-three patterns (X = color, . = empty, | = edge/opponent):
	// Type A: . X X X .     (3 consecutive, both ends open)
	// Type B: . X X . X .   (gap between 2nd and 3rd)
	// Type C: . X . X X .   (gap between 1st and 2nd)

	// Gather cells along this direction centered around pos
	// We look at positions from -4 to +4 relative to pos
	Cell line[9]; // positions -4 to +4
	for (int i = 0; i < 9; i++) {
		int nr = r + (i - 4) * dr;
		int nc = c + (i - 4) * dc;
		if (!in_bounds(nr, nc))
			line[i] = (Cell)-1; // out of bounds = blocked (sentinel)
		else {
			int p = idx(nr, nc);
			if (p == pos)
				line[i] = color; // pretend stone is placed
			else
				line[i] = (Cell)board[p];
		}
	}
	// The placed stone is at index 4 in line[]

	// Now scan for free-three patterns that include index 4 (the placed stone)
	// and have exactly 3 stones of color

	// Type A: . X X X .  (5 cells window)
	// Check all windows of size 5 that contain index 4
	for (int start = 0; start <= 4; start++) {
		int end = start + 4; // inclusive
		if (end >= 9) break;
		if (start + 4 < 4 || start > 4) continue; // must include pos (index 4)

		// Check: empty, 3x color, empty
		if (line[start] != EMPTY || line[end] != EMPTY)
			continue;

		int count = 0;
		bool has_pos = false;
		bool valid = true;
		for (int j = start + 1; j < end; j++) {
			if (line[j] == color) {
				count++;
				if (j == 4) has_pos = true;
			} else {
				valid = false;
				break;
			}
		}
		if (valid && count == 3 && has_pos) {
			// Also check that the cells beyond the ends are not extending the line
			// (if they are same color, this would be a four, not a three)
			bool before_ok = (start == 0) || (line[start - 1] != color);
			bool after_ok = (end == 8) || (line[end + 1] != color);
			if (before_ok && after_ok)
				return true;
		}
	}

	// Type B: . X X . X .  (6 cells window)
	// Type C: . X . X X .  (6 cells window)
	for (int start = 0; start <= 3; start++) {
		int end = start + 5; // inclusive
		if (end >= 9) break;
		if (start > 4 || end < 4) continue; // must include pos (index 4)

		if (line[start] != EMPTY || line[end] != EMPTY)
			continue;

		// Count stones and gaps in the inner 4 cells (start+1 to end-1)
		int count = 0;
		int gaps = 0;
		bool has_pos = false;
		bool valid = true;
		for (int j = start + 1; j < end; j++) {
			if (line[j] == color) {
				count++;
				if (j == 4) has_pos = true;
			} else if (line[j] == EMPTY) {
				gaps++;
			} else {
				valid = false;
				break;
			}
		}
		if (valid && count == 3 && gaps == 1 && has_pos)
			return true;
	}

	return false;
}

// Count how many free-threes placing color at pos would create
int count_free_threes(const int8_t *board, int pos, Cell color) {
	int count = 0;
	for (int d = 0; d < 4; d++) {
		if (is_free_three(board, pos, color, DIR[d][0], DIR[d][1]))
			count++;
	}
	return count;
}

// Check if a move is legal (empty + not double-three, or double-three with capture)
bool is_legal_move(const GameState &s, int pos) {
	if (pos < 0 || pos >= BCELLS || s.board[pos] != EMPTY)
		return false;

	int ft = count_free_threes(s.board, pos, s.current);
	if (ft >= 2) {
		// Exception: if this move captures at least one pair, it's legal
		// Temporarily place stone to check captures, then remove it
		GameState &ms = const_cast<GameState&>(s);
		ms.board[pos] = s.current;
		int caps = check_captures(ms, pos, s.current);
		ms.board[pos] = EMPTY;
		if (caps > 0)
			return true;
		return false;
	}
	return true;
}

// Check if opponent can break a five-in-a-row by capturing
bool can_opponent_break_five(const GameState &s, int pos, Cell color) {
	Cell opp = opponent(color);

	// Get the positions of the five-in-a-row
	int five_pos[BSIZE];
	int five_count = 0;
	get_five_positions(s.board, pos, color, five_pos, five_count);
	if (five_count < 5) return false;

	// Try every empty cell as a potential opponent move
	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] != EMPTY) continue;

		// Simulate opponent placing at i
		GameState tmp = s;
		tmp.board[i] = opp;

		// Check if this placement captures any pair
		int r = row_of(i), c = col_of(i);
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
				if (tmp.board[i1] == color && tmp.board[i2] == color && tmp.board[i3] == opp) {
					// This captures a pair of 'color' stones
					// Check if either captured stone is part of the five
					for (int f = 0; f < five_count; f++) {
						if (five_pos[f] == i1 || five_pos[f] == i2)
							return true; // can break the five
					}
					// Also check: does this capture give opponent 5 pairs?
					int opp_caps = s.captures[opp - 1] + 1;
					if (opp_caps >= 5)
						return true; // opponent wins by capture instead
				}
			}
		}
	}
	return false;
}
