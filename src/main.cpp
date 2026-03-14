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

	bool quit = false;
	while (!quit) {
		gui_poll_events(state, gui, quit);
		if (quit) break;

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
