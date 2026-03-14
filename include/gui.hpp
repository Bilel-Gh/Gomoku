#pragma once

#include "gomoku.hpp"
#include <SDL2/SDL.h>

struct Color {
	uint8_t r, g, b, a;
};

struct Theme {
	Color board_bg;
	Color line_color;
	Color text_color;
	Color panel_bg;
	Color black_stone;
	Color white_stone;
	Color highlight;
	Color btn_bg;
	Color btn_text;
};

struct GuiState {
	GameMode   mode;
	Cell       ai_color;       // which side is AI (STONE_WHITE by default)
	bool       ai_thinking;
	double     ai_time;        // last AI computation time
	bool       show_hint;
	int        hint_move;      // AI suggested move for human
	bool       show_heatmap;
	int        theme_idx;      // 0 = light, 1 = dark
	std::vector<Snapshot> undo_stack;
	std::vector<Snapshot> redo_stack;
};

void gui_init();
void gui_draw(const GameState &s, const GuiState &g);
int  gui_handle_input(GameState &s, GuiState &g);
void gui_close();
bool gui_should_close();
void gui_poll_events(GameState &s, GuiState &g, bool &quit);

// Convert board position to screen coords
inline int board_x(int c) { return BOARD_MARGIN + c * CELL_SIZE; }
inline int board_y(int r) { return BOARD_MARGIN + r * CELL_SIZE; }
