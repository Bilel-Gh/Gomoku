#include "gomoku.hpp"
#include "ai.hpp"

int pattern_score(PatternType type) {
	switch (type) {
		case PAT_FIVE:            return SCORE_FIVE;
		case PAT_OPEN_FOUR:       return SCORE_OPEN_FOUR;
		case PAT_HALF_OPEN_FOUR:  return SCORE_HALF_OPEN_FOUR;
		case PAT_GAP_FOUR:        return SCORE_GAP_FOUR;
		case PAT_OPEN_THREE:      return SCORE_OPEN_THREE;
		case PAT_GAP_THREE:       return SCORE_GAP_THREE;
		case PAT_HALF_OPEN_THREE: return SCORE_HALF_OPEN_THREE;
		case PAT_OPEN_TWO:        return SCORE_OPEN_TWO;
		case PAT_HALF_OPEN_TWO:   return SCORE_HALF_OPEN_TWO;
		default:                  return 0;
	}
}

PatternType classify_pattern(int count, int open_ends, int gaps) {
	if (count >= 5) return PAT_FIVE;

	if (count == 4) {
		if (gaps == 0) {
			if (open_ends == 2) return PAT_OPEN_FOUR;
			if (open_ends == 1) return PAT_HALF_OPEN_FOUR;
		} else { // gaps == 1
			if (open_ends >= 1) return PAT_GAP_FOUR;
		}
		return PAT_NONE;
	}

	if (count == 3) {
		if (gaps == 0) {
			if (open_ends == 2) return PAT_OPEN_THREE;
			if (open_ends == 1) return PAT_HALF_OPEN_THREE;
		} else { // gaps == 1
			if (open_ends == 2) return PAT_GAP_THREE;
			if (open_ends == 1) return PAT_HALF_OPEN_THREE;
		}
		return PAT_NONE;
	}

	if (count == 2) {
		if (open_ends == 2) return PAT_OPEN_TWO;
		if (open_ends == 1) return PAT_HALF_OPEN_TWO;
	}

	return PAT_NONE;
}

// Scan a pattern starting from pos in direction (dr, dc)
// Returns the pattern type for the group of stones containing pos
static PatternType scan_line(const int8_t *board, int pos, int dr, int dc) {
	Cell color = (Cell)board[pos];
	if (color == EMPTY) return PAT_NONE;

	int r = row_of(pos), c = col_of(pos);
	int count = 1;
	int gaps = 0;
	// Walk in positive direction
	bool gap_used = false;
	for (int step = 1; step < 5; step++) {
		int nr = r + step * dr;
		int nc = c + step * dc;
		if (!in_bounds(nr, nc)) break;
		Cell cell = (Cell)board[idx(nr, nc)];
		if (cell == color) {
			count++;
		} else if (cell == EMPTY && !gap_used && step < 4) {
			// Check if stone after the gap continues
			int nr2 = nr + dr, nc2 = nc + dc;
			if (in_bounds(nr2, nc2) && (Cell)board[idx(nr2, nc2)] == color) {
				gaps = 1;
				gap_used = true;
				// Don't count the gap, but continue
			} else {
				break;
			}
		} else {
			break;
		}
	}

	// Check open ends
	int open_ends = 0;
	// Before the start
	int br = r - dr, bc = c - dc;
	if (in_bounds(br, bc) && board[idx(br, bc)] == EMPTY)
		open_ends++;
	// After the end: walk past all counted stones (and gaps)
	int total_span = count + gaps;
	int er = r + total_span * dr;
	int ec = c + total_span * dc;
	if (in_bounds(er, ec) && board[idx(er, ec)] == EMPTY)
		open_ends++;

	return classify_pattern(count, open_ends, gaps);
}

// Full board evaluation
int evaluate(const GameState &s, Cell ai_color) {
	int score = 0;
	Cell opp_color = opponent(ai_color);
	bool ai_turn = (s.current == ai_color);

	// Track which cells we already scanned in each direction to avoid double-counting
	bool scanned[BCELLS][4] = {};

	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] == EMPTY) continue;
		Cell color = (Cell)s.board[i];

		for (int d = 0; d < 4; d++) {
			if (scanned[i][d]) continue;

			// Only scan from the "start" of a group (first stone in negative direction)
			int r = row_of(i), c = col_of(i);
			int pr = r - DIR[d][0], pc = c - DIR[d][1];
			if (in_bounds(pr, pc) && s.board[idx(pr, pc)] == color) {
				// Not the start of the group, skip
				scanned[i][d] = true;
				continue;
			}

			// Scan the pattern
			PatternType pat = scan_line(s.board, i, DIR[d][0], DIR[d][1]);

			// Mark all cells in this group as scanned
			int nr = r, nc = c;
			while (in_bounds(nr, nc) && s.board[idx(nr, nc)] == color) {
				scanned[idx(nr, nc)][d] = true;
				nr += DIR[d][0];
				nc += DIR[d][1];
			}

			if (pat == PAT_NONE) continue;

			int ps = pattern_score(pat);

			// Turn-dependent scaling
			bool is_my_turn = (color == ai_color) ? ai_turn : !ai_turn;
			if (is_my_turn) {
				if (pat == PAT_OPEN_THREE || pat == PAT_GAP_THREE)
					ps *= TURN_MULT_THREE;
				else if (pat == PAT_HALF_OPEN_FOUR || pat == PAT_GAP_FOUR)
					ps *= TURN_MULT_FOUR;
			}

			if (color == ai_color)
				score += ps;
			else
				score -= ps;
		}
	}

	// Capture scoring
	int ai_caps = s.captures[ai_color - 1];
	int opp_caps = s.captures[opp_color - 1];
	score += CAPTURE_VALUES[ai_caps < 5 ? ai_caps : 5];
	score -= CAPTURE_VALUES[opp_caps < 5 ? opp_caps : 5];

	if (ai_caps >= 4)
		score += count_capturable_pairs(s.board, opp_color) * CAPTURE_THREAT_BONUS;
	if (opp_caps >= 4)
		score -= count_capturable_pairs(s.board, ai_color) * CAPTURE_THREAT_BONUS;

	// Tempo bonus
	if (ai_turn)
		score += TEMPO_BONUS;
	else
		score -= TEMPO_BONUS;

	return score;
}

// Count pairs of 'target_color' that can be captured by opponent
int count_capturable_pairs(const int8_t *board, Cell target_color) {
	Cell attacker = opponent(target_color);
	int count = 0;

	for (int i = 0; i < BCELLS; i++) {
		if (board[i] != target_color) continue;
		int r = row_of(i), c = col_of(i);

		for (int d = 0; d < 4; d++) {
			int dr = DIR[d][0], dc = DIR[d][1];
			// Check pattern: EMPTY target target attacker (or attacker target target EMPTY)
			int r2 = r + dr, c2 = c + dc;
			if (!in_bounds(r2, c2) || board[idx(r2, c2)] != target_color) continue;

			// Check flanks
			int rb = r - dr, cb = c - dc; // before
			int ra = r2 + dr, ca = c2 + dc; // after
			if (in_bounds(rb, cb) && board[idx(rb, cb)] == EMPTY &&
				in_bounds(ra, ca) && board[idx(ra, ca)] == attacker)
				count++;
			if (in_bounds(rb, cb) && board[idx(rb, cb)] == attacker &&
				in_bounds(ra, ca) && board[idx(ra, ca)] == EMPTY)
				count++;
		}
	}
	return count / 2; // each pair counted twice
}

// Quick evaluation of a single move (for move ordering)
int quick_eval_move(const GameState &s, int pos, Cell color) {
	int score = 0;
	int r = row_of(pos), c = col_of(pos);

	for (int d = 0; d < 4; d++) {
		int dr = DIR[d][0], dc = DIR[d][1];
		int count = 1;
		int open_ends = 0;

		// Positive direction
		for (int step = 1; step < 5; step++) {
			int nr = r + step * dr, nc = c + step * dc;
			if (!in_bounds(nr, nc) || s.board[idx(nr, nc)] != color) {
				if (in_bounds(nr, nc) && s.board[idx(nr, nc)] == EMPTY)
					open_ends++;
				break;
			}
			count++;
		}
		// Negative direction
		for (int step = 1; step < 5; step++) {
			int nr = r - step * dr, nc = c - step * dc;
			if (!in_bounds(nr, nc) || s.board[idx(nr, nc)] != color) {
				if (in_bounds(nr, nc) && s.board[idx(nr, nc)] == EMPTY)
					open_ends++;
				break;
			}
			count++;
		}

		PatternType pat = classify_pattern(count, open_ends, 0);
		score += pattern_score(pat);
	}
	return score;
}
