#pragma once

// -- Board --
constexpr int BSIZE  = 19;
constexpr int BCELLS = BSIZE * BSIZE; // 361

// -- Pattern scores (alignment threats) --
constexpr int SCORE_FIVE            = 100000000;
constexpr int SCORE_OPEN_FOUR       = 10000000;
constexpr int SCORE_HALF_OPEN_FOUR  = 500000;
constexpr int SCORE_GAP_FOUR        = 450000;
constexpr int SCORE_OPEN_THREE      = 50000;
constexpr int SCORE_GAP_THREE       = 40000;
constexpr int SCORE_HALF_OPEN_THREE = 5000;
constexpr int SCORE_OPEN_TWO        = 3000;
constexpr int SCORE_HALF_OPEN_TWO   = 500;

// -- Turn-dependent multipliers --
constexpr int TURN_MULT_THREE       = 8;
constexpr int TURN_MULT_FOUR        = 20;

// -- Capture scoring (escalating) --
constexpr int CAPTURE_VALUES[]      = {0, 5000, 15000, 50000, 200000, 100000000};
constexpr int CAPTURE_THREAT_BONUS  = 1000000;

// -- Move ordering priorities --
constexpr int ORDER_WIN             = 10000000;
constexpr int ORDER_TT_MOVE         = 5000000;
constexpr int ORDER_CAPTURE         = 1000000;
constexpr int ORDER_THREAT          = 500000;
constexpr int ORDER_BLOCK           = 400000;

// -- Search parameters --
constexpr int TT_SIZE_BITS          = 20;
constexpr int TT_SIZE               = 1 << TT_SIZE_BITS;
constexpr int TIME_CHECK_INTERVAL   = 1024;
constexpr double TIME_LIMIT         = 0.45;
constexpr double TIME_SOFT_LIMIT    = 0.40;
constexpr int CANDIDATE_DISTANCE    = 2;

// -- Misc eval --
constexpr int TEMPO_BONUS           = 1000;
constexpr int WIN_SCORE             = 99999999;

// -- GUI constants --
constexpr int CELL_SIZE             = 40;
constexpr int BOARD_MARGIN          = 30;
constexpr int BOARD_PX              = BOARD_MARGIN * 2 + (BSIZE - 1) * CELL_SIZE;
constexpr int PANEL_WIDTH           = 300;
constexpr int WINDOW_W              = BOARD_PX + PANEL_WIDTH;
constexpr int WINDOW_H              = BOARD_PX;
constexpr int STONE_RADIUS          = 17;
constexpr int FPS                   = 60;

// Direction vectors (4 axes)
constexpr int DIR[4][2] = {
	{0, 1},   // horizontal
	{1, 0},   // vertical
	{1, 1},   // diagonal
	{1, -1}   // anti-diagonal
};
