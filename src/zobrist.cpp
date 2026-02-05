#include "gomoku.hpp"
#include <random>
#include <cstring>

uint64_t zobrist_table[BCELLS][3];
TTEntry  tt[TT_SIZE];
uint16_t tt_generation = 0;

void init_zobrist() {
	std::mt19937_64 rng(42); // fixed seed
	for (int i = 0; i < BCELLS; i++) {
		zobrist_table[i][0] = 0; // EMPTY not used
		zobrist_table[i][1] = rng();
		zobrist_table[i][2] = rng();
	}
	tt_clear();
}

TTEntry* tt_lookup(uint64_t hash) {
	int i = hash & (TT_SIZE - 1);
	if (tt[i].key == hash)
		return &tt[i];
	return nullptr;
}

void tt_store(uint64_t hash, int depth, int score, TTFlag flag, int best_move) {
	int i = hash & (TT_SIZE - 1);
	// Replace if empty, same hash, shallower, or old
	if (tt[i].key == 0 || tt[i].key == hash ||
		tt[i].depth <= depth || tt[i].age != tt_generation) {
		tt[i].key = hash;
		tt[i].depth = depth;
		tt[i].score = score;
		tt[i].flag = flag;
		tt[i].best_move = best_move;
		tt[i].age = tt_generation;
	}
}

void tt_clear() {
	memset(tt, 0, sizeof(tt));
	tt_generation = 0;
}
