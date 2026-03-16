#pragma once

#include <cstdint>
#include <vector>
#include "constants.hpp"

enum Cell : int8_t {
	EMPTY  = 0,
	STONE_BLACK = 1,
	STONE_WHITE = 2
};

enum PatternType {
	PAT_NONE = 0,
	PAT_FIVE,
	PAT_OPEN_FOUR,
	PAT_HALF_OPEN_FOUR,
	PAT_GAP_FOUR,
	PAT_OPEN_THREE,
	PAT_GAP_THREE,
	PAT_HALF_OPEN_THREE,
	PAT_OPEN_TWO,
	PAT_HALF_OPEN_TWO,
	PAT_COUNT
};

enum TTFlag : uint8_t {
	TT_EXACT,
	TT_ALPHA,
	TT_BETA
};

enum GameMode {
	MODE_PVA, // player vs AI
	MODE_PVP  // player vs player (hotseat)
};

inline int idx(int r, int c) { return r * BSIZE + c; }
inline int row_of(int i) { return i / BSIZE; }
inline int col_of(int i) { return i % BSIZE; }
inline bool in_bounds(int r, int c) { return r >= 0 && r < BSIZE && c >= 0 && c < BSIZE; }
inline Cell opponent(Cell c) { return (c == STONE_BLACK) ? STONE_WHITE : STONE_BLACK; }

struct GameState {
	int8_t    board[BCELLS];
	int       captures[2];  // captures[0] = STONE_BLACK pairs, captures[1] = STONE_WHITE pairs
	Cell      current;
	int       move_count;
	int       last_move;
	uint64_t  zhash;
	bool      game_over;
	Cell      winner;
	int       pending_five_pos;   // position of unbroken five awaiting opponent response (-1 = none)
	Cell      pending_five_color; // color that made the pending five
};

struct Move {
	int pos;
	int score;
};

struct MoveUndo {
	int      pos;
	int      captured[8]; // up to 4 pairs = 8 stones
	int      num_captured;
	uint64_t prev_hash;
	Cell     prev_current;
	int      prev_move_count;
	int      prev_last_move;
	int      prev_pending_five_pos;
	Cell     prev_pending_five_color;
};

struct TTEntry {
	uint64_t key;
	int      depth;
	int      score;
	int      best_move;
	TTFlag   flag;
	uint16_t age;
};

struct Snapshot {
	int8_t   board[BCELLS];
	int      captures[2];
	Cell     current;
	int      move_count;
	int      last_move;
	uint64_t zhash;
	bool     game_over;
	Cell     winner;
	int      pending_five_pos;
	Cell     pending_five_color;
};

// board.cpp
void     init_game(GameState &s);
bool     place_stone(GameState &s, int pos);
int      check_captures(const GameState &s, int pos, Cell color);
void     apply_captures(GameState &s, int pos, Cell color, MoveUndo &undo);
bool     check_five(const GameState &s, int pos, Cell color);
bool     check_win(GameState &s, int pos, Cell color);
void     get_five_positions(const int8_t *board, int pos, Cell color, int *positions, int &count);

// rules.cpp
bool     is_free_three(const int8_t *board, int pos, Cell color, int dr, int dc);
int      count_free_threes(const int8_t *board, int pos, Cell color);
bool     is_legal_move(const GameState &s, int pos);
bool     can_opponent_break_five(const GameState &s, int pos, Cell color);

// ai.cpp - apply/undo for search
void     ai_apply_move(GameState &s, int pos, MoveUndo &undo);
void     ai_undo_move(GameState &s, MoveUndo &undo);
int      ai_get_move(GameState &s, Cell ai_color, double &elapsed_out);
int      ai_suggest_move(GameState &s, Cell color);

// zobrist.cpp
void     init_zobrist();
extern uint64_t zobrist_table[BCELLS][3];
extern TTEntry  tt[];
extern uint16_t tt_generation;
TTEntry* tt_lookup(uint64_t hash);
void     tt_store(uint64_t hash, int depth, int score, TTFlag flag, int best_move);
void     tt_clear();

// eval.cpp
int      evaluate(const GameState &s, Cell ai_color);
int      pattern_score(PatternType type);
PatternType classify_pattern(int count, int open_ends, int gaps);

// snapshot helpers
Snapshot make_snapshot(const GameState &s);
void     restore_snapshot(GameState &s, const Snapshot &snap);
