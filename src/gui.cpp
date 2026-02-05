#include "gui.hpp"
#include "ai.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

static Theme themes[] = {
	// Light theme (wood)
	{
		{220, 179, 92, 255},    // board_bg
		{40, 30, 10, 255},      // line_color
		{20, 20, 20, 255},      // text_color
		{240, 230, 210, 255},   // panel_bg
		{20, 20, 20, 255},      // black_stone
		{235, 235, 235, 255},   // white_stone
		{255, 50, 50, 255},     // highlight
		{180, 140, 60, 255},    // btn_bg
		{255, 255, 255, 255}    // btn_text
	},
	// Dark theme
	{
		{40, 42, 54, 255},
		{100, 100, 120, 255},
		{220, 220, 220, 255},
		{30, 30, 40, 255},
		{200, 200, 220, 255},
		{60, 60, 80, 255},
		{80, 200, 120, 255},
		{60, 60, 80, 255},
		{220, 220, 220, 255}
	}
};

// Star points (hoshi) on a 19x19 board
static const int hoshi[][2] = {
	{3,3}, {3,9}, {3,15},
	{9,3}, {9,9}, {9,15},
	{15,3}, {15,9}, {15,15}
};

void gui_init() {
	InitWindow(WINDOW_W, WINDOW_H, "Gomoku");
	SetTargetFPS(FPS);
}

void gui_close() {
	CloseWindow();
}

static void draw_board(const GameState &s, const GuiState &g) {
	const Theme &t = themes[g.theme_idx];

	// Board background
	DrawRectangle(0, 0, BOARD_PX, BOARD_PX, t.board_bg);

	// Grid lines
	for (int i = 0; i < BSIZE; i++) {
		int x = board_x(i);
		int y = board_y(i);
		DrawLine(x, board_y(0), x, board_y(BSIZE - 1), t.line_color);
		DrawLine(board_x(0), y, board_x(BSIZE - 1), y, t.line_color);
	}

	// Star points
	for (int i = 0; i < 9; i++)
		DrawCircle(board_x(hoshi[i][1]), board_y(hoshi[i][0]), 4, t.line_color);

	// Stones
	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] == EMPTY) continue;
		int x = board_x(col_of(i));
		int y = board_y(row_of(i));
		Color c = (s.board[i] == STONE_BLACK) ? t.black_stone : t.white_stone;
		DrawCircle(x, y, STONE_RADIUS, c);
		// Outline for white stones (visibility)
		if (s.board[i] == STONE_WHITE)
			DrawCircleLines(x, y, STONE_RADIUS, t.line_color);
	}

	// Last move indicator
	if (s.last_move >= 0) {
		int x = board_x(col_of(s.last_move));
		int y = board_y(row_of(s.last_move));
		DrawCircle(x, y, 5, t.highlight);
	}

	// Hover preview (when human's turn)
	if (!s.game_over && !g.ai_thinking) {
		Vector2 mouse = GetMousePosition();
		int mc = (int)roundf((mouse.x - BOARD_MARGIN) / (float)CELL_SIZE);
		int mr = (int)roundf((mouse.y - BOARD_MARGIN) / (float)CELL_SIZE);
		if (in_bounds(mr, mc) && s.board[idx(mr, mc)] == EMPTY) {
			Color preview = (s.current == STONE_BLACK) ? t.black_stone : t.white_stone;
			preview.a = 100;
			DrawCircle(board_x(mc), board_y(mr), STONE_RADIUS, preview);
		}
	}

	// Hint overlay
	if (g.show_hint && g.hint_move >= 0 && !s.game_over) {
		int x = board_x(col_of(g.hint_move));
		int y = board_y(row_of(g.hint_move));
		DrawCircle(x, y, STONE_RADIUS - 2, Fade(GREEN, 0.4f));
		DrawText("H", x - 5, y - 7, 16, WHITE);
	}
}

static void draw_heatmap(const GameState &s, const GuiState &g) {
	if (!g.show_heatmap) return;
	const Theme &t = themes[g.theme_idx];

	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] != EMPTY) continue;

		// Check if within candidate range
		bool near = false;
		int r = row_of(i), c = col_of(i);
		for (int dr = -CANDIDATE_DISTANCE; dr <= CANDIDATE_DISTANCE && !near; dr++) {
			for (int dc = -CANDIDATE_DISTANCE; dc <= CANDIDATE_DISTANCE && !near; dc++) {
				int nr = r + dr, nc = c + dc;
				if (in_bounds(nr, nc) && s.board[idx(nr, nc)] != EMPTY)
					near = true;
			}
		}
		if (!near) continue;

		int score = quick_eval_move(s, i, s.current);
		int x = board_x(c);
		int y = board_y(r);

		// Map score to color
		float intensity = (float)score / 100000.0f;
		if (intensity > 1.0f) intensity = 1.0f;
		if (intensity < 0.0f) intensity = 0.0f;
		Color heat = {
			(unsigned char)(255 * (1.0f - intensity)),
			(unsigned char)(255 * intensity),
			0, 120
		};
		DrawRectangle(x - CELL_SIZE/2, y - CELL_SIZE/2, CELL_SIZE, CELL_SIZE, heat);

		char buf[16];
		snprintf(buf, sizeof(buf), "%d", score / 1000);
		DrawText(buf, x - 10, y - 6, 10, t.text_color);
	}
}

static void draw_panel(const GameState &s, const GuiState &g) {
	const Theme &t = themes[g.theme_idx];
	int px = BOARD_PX;
	int pw = PANEL_WIDTH;

	DrawRectangle(px, 0, pw, WINDOW_H, t.panel_bg);

	int y = 20;
	DrawText("GOMOKU", px + 20, y, 30, t.text_color);
	y += 50;

	// Turn
	const char *turn_str = s.game_over
		? (s.winner == EMPTY ? "Draw!" : (s.winner == STONE_BLACK ? "Black wins!" : "White wins!"))
		: (s.current == STONE_BLACK ? "Turn: Black" : "Turn: White");
	DrawText(turn_str, px + 20, y, 20, t.text_color);
	y += 35;

	// AI thinking
	if (g.ai_thinking) {
		DrawText("AI thinking...", px + 20, y, 18, t.highlight);
	} else if (g.ai_time > 0) {
		char buf[64];
		snprintf(buf, sizeof(buf), "AI time: %.3fs", g.ai_time);
		DrawText(buf, px + 20, y, 18, t.text_color);
	}
	y += 35;

	// Captures
	char cap_buf[64];
	snprintf(cap_buf, sizeof(cap_buf), "Black captures: %d", s.captures[0]);
	DrawText(cap_buf, px + 20, y, 18, t.text_color);
	y += 25;
	snprintf(cap_buf, sizeof(cap_buf), "White captures: %d", s.captures[1]);
	DrawText(cap_buf, px + 20, y, 18, t.text_color);
	y += 35;

	// Move count
	char mc_buf[32];
	snprintf(mc_buf, sizeof(mc_buf), "Moves: %d", s.move_count);
	DrawText(mc_buf, px + 20, y, 18, t.text_color);
	y += 40;

	// Mode
	const char *mode_str = (g.mode == MODE_PVA) ? "Mode: vs AI" : "Mode: vs Human";
	DrawText(mode_str, px + 20, y, 18, t.text_color);
	y += 40;

	// Buttons
	int btn_w = pw - 40;
	int btn_h = 35;

	// New Game button
	Rectangle btn_new = {(float)(px + 20), (float)y, (float)btn_w, (float)btn_h};
	DrawRectangleRec(btn_new, t.btn_bg);
	DrawText("New Game (N)", px + 30, y + 8, 18, t.btn_text);
	y += btn_h + 10;

	// Toggle Mode button
	Rectangle btn_mode = {(float)(px + 20), (float)y, (float)btn_w, (float)btn_h};
	DrawRectangleRec(btn_mode, t.btn_bg);
	const char *toggle_str = (g.mode == MODE_PVA) ? "Switch to PvP (M)" : "Switch to PvAI (M)";
	DrawText(toggle_str, px + 30, y + 8, 18, t.btn_text);
	y += btn_h + 10;

	// Undo button
	Rectangle btn_undo = {(float)(px + 20), (float)y, (float)btn_w, (float)btn_h};
	DrawRectangleRec(btn_undo, t.btn_bg);
	DrawText("Undo (Z)", px + 30, y + 8, 18, t.btn_text);
	y += btn_h + 10;

	// Redo button
	Rectangle btn_redo = {(float)(px + 20), (float)y, (float)btn_w, (float)btn_h};
	DrawRectangleRec(btn_redo, t.btn_bg);
	DrawText("Redo (Y)", px + 30, y + 8, 18, t.btn_text);
	y += btn_h + 20;

	// Shortcuts
	DrawText("Shortcuts:", px + 20, y, 16, t.text_color);
	y += 22;
	DrawText("H - Hint", px + 20, y, 14, t.text_color);
	y += 18;
	DrawText("T - Theme", px + 20, y, 14, t.text_color);
	y += 18;
	DrawText("D - Heatmap", px + 20, y, 14, t.text_color);
	y += 18;
	DrawText("N - New Game", px + 20, y, 14, t.text_color);
	y += 18;
	DrawText("M - Toggle Mode", px + 20, y, 14, t.text_color);
}

void gui_draw(const GameState &s, const GuiState &g) {
	BeginDrawing();
	ClearBackground(themes[g.theme_idx].panel_bg);
	draw_board(s, g);
	draw_heatmap(s, g);
	draw_panel(s, g);
	EndDrawing();
}

// Returns: board position clicked, or -1 if nothing relevant
int gui_handle_input(GameState &s, GuiState &g) {
	// Mouse click on board
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !s.game_over && !g.ai_thinking) {
		Vector2 mouse = GetMousePosition();
		int mc = (int)roundf((mouse.x - BOARD_MARGIN) / (float)CELL_SIZE);
		int mr = (int)roundf((mouse.y - BOARD_MARGIN) / (float)CELL_SIZE);
		if (in_bounds(mr, mc)) {
			int pos = idx(mr, mc);
			if (s.board[pos] == EMPTY && is_legal_move(s, pos))
				return pos;
		}
	}
	return -1;
}
