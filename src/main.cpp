#include "gomoku.hpp"
#include "ai.hpp"
#include "gui.hpp"
#include <iostream>
#include <cstring>

int main() {
	init_zobrist();

	GameState state;
	init_game(state);

	GuiState gui;
	memset(&gui, 0, sizeof(gui));
	gui.mode = MODE_PVA;
	gui.ai_color = STONE_WHITE;
	gui.hint_move = -1;
	gui.theme_idx = 0;

	gui_init();

	while (!WindowShouldClose()) {
		// Keyboard shortcuts
		if (IsKeyPressed(KEY_N)) {
			// New game
			init_game(state);
			gui.undo_stack.clear();
			gui.redo_stack.clear();
			gui.ai_thinking = false;
			gui.ai_time = 0;
			gui.hint_move = -1;
			tt_clear();
		}
		if (IsKeyPressed(KEY_M)) {
			// Toggle mode
			gui.mode = (gui.mode == MODE_PVA) ? MODE_PVP : MODE_PVA;
			init_game(state);
			gui.undo_stack.clear();
			gui.redo_stack.clear();
			gui.ai_thinking = false;
			gui.ai_time = 0;
			gui.hint_move = -1;
			tt_clear();
		}
		if (IsKeyPressed(KEY_T)) {
			gui.theme_idx = (gui.theme_idx + 1) % 2;
		}
		if (IsKeyPressed(KEY_H)) {
			gui.show_hint = !gui.show_hint;
			if (gui.show_hint && !state.game_over) {
				gui.hint_move = ai_suggest_move(state, state.current);
			}
		}
		if (IsKeyPressed(KEY_D)) {
			gui.show_heatmap = !gui.show_heatmap;
		}

		// Undo (Z)
		if (IsKeyPressed(KEY_Z) && !gui.undo_stack.empty() && !gui.ai_thinking) {
			gui.redo_stack.push_back(make_snapshot(state));
			restore_snapshot(state, gui.undo_stack.back());
			gui.undo_stack.pop_back();
			// In PvA mode, undo twice (undo AI move + undo player move)
			if (gui.mode == MODE_PVA && !gui.undo_stack.empty()) {
				gui.redo_stack.push_back(make_snapshot(state));
				restore_snapshot(state, gui.undo_stack.back());
				gui.undo_stack.pop_back();
			}
			gui.hint_move = -1;
		}

		// Redo (Y)
		if (IsKeyPressed(KEY_Y) && !gui.redo_stack.empty() && !gui.ai_thinking) {
			gui.undo_stack.push_back(make_snapshot(state));
			restore_snapshot(state, gui.redo_stack.back());
			gui.redo_stack.pop_back();
			if (gui.mode == MODE_PVA && !gui.redo_stack.empty()) {
				gui.undo_stack.push_back(make_snapshot(state));
				restore_snapshot(state, gui.redo_stack.back());
				gui.redo_stack.pop_back();
			}
			gui.hint_move = -1;
		}

		// Human turn
		bool human_turn = !state.game_over && !gui.ai_thinking;
		if (gui.mode == MODE_PVA)
			human_turn = human_turn && (state.current != gui.ai_color);

		if (human_turn) {
			int clicked = gui_handle_input(state, gui);
			if (clicked >= 0) {
				gui.undo_stack.push_back(make_snapshot(state));
				gui.redo_stack.clear();
				place_stone(state, clicked);
				gui.hint_move = -1;
			}
		}

		// AI turn (in PvA mode)
		if (gui.mode == MODE_PVA && !state.game_over &&
			state.current == gui.ai_color && !gui.ai_thinking) {
			gui.ai_thinking = true;
			// Draw one frame to show "AI thinking" before blocking
			gui_draw(state, gui);

			double elapsed = 0;
			int ai_move = ai_get_move(state, gui.ai_color, elapsed);
			gui.ai_time = elapsed;
			gui.ai_thinking = false;

			if (ai_move >= 0) {
				gui.undo_stack.push_back(make_snapshot(state));
				gui.redo_stack.clear();
				place_stone(state, ai_move);
			}
			gui.hint_move = -1;
		}

		gui_draw(state, gui);
	}

	gui_close();
	return 0;
}
