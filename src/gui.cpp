#include "gui.hpp"
#include "ai.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

// ── Global SDL state ──
static SDL_Window   *g_window   = nullptr;
static SDL_Renderer *g_renderer = nullptr;
static bool          g_mouse_pressed = false;
static int           g_mouse_x = 0, g_mouse_y = 0;
static const uint8_t *g_keys = nullptr;
static bool          g_key_just[SDL_NUM_SCANCODES] = {};

// ── Themes ──
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

// ── Minimal 5x7 bitmap font ──
// Each char is 5 columns x 7 rows, stored as 7 bytes (each byte = 5 bits, MSB left)
static const uint8_t font5x7[][7] = {
	// ' ' (32)
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	// '!' (33)
	{0x04,0x04,0x04,0x04,0x04,0x00,0x04},
	// '"' (34)
	{0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},
	// '#' (35)
	{0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00},
	// '$' (36)
	{0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
	// '%' (37)
	{0x19,0x19,0x02,0x04,0x08,0x13,0x13},
	// '&' (38)
	{0x08,0x14,0x14,0x08,0x15,0x12,0x0D},
	// '\'' (39)
	{0x04,0x04,0x00,0x00,0x00,0x00,0x00},
	// '(' (40)
	{0x02,0x04,0x08,0x08,0x08,0x04,0x02},
	// ')' (41)
	{0x08,0x04,0x02,0x02,0x02,0x04,0x08},
	// '*' (42)
	{0x00,0x04,0x15,0x0E,0x15,0x04,0x00},
	// '+' (43)
	{0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
	// ',' (44)
	{0x00,0x00,0x00,0x00,0x00,0x04,0x08},
	// '-' (45)
	{0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
	// '.' (46)
	{0x00,0x00,0x00,0x00,0x00,0x00,0x04},
	// '/' (47)
	{0x01,0x01,0x02,0x04,0x08,0x10,0x10},
	// '0' (48)
	{0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
	// '1' (49)
	{0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
	// '2' (50)
	{0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
	// '3' (51)
	{0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
	// '4' (52)
	{0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
	// '5' (53)
	{0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
	// '6' (54)
	{0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
	// '7' (55)
	{0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
	// '8' (56)
	{0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
	// '9' (57)
	{0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
	// ':' (58)
	{0x00,0x00,0x04,0x00,0x04,0x00,0x00},
	// ';' (59)
	{0x00,0x00,0x04,0x00,0x04,0x04,0x08},
	// '<' (60)
	{0x02,0x04,0x08,0x10,0x08,0x04,0x02},
	// '=' (61)
	{0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
	// '>' (62)
	{0x08,0x04,0x02,0x01,0x02,0x04,0x08},
	// '?' (63)
	{0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
	// '@' (64)
	{0x0E,0x11,0x17,0x15,0x17,0x10,0x0E},
	// A-Z (65-90)
	{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // A
	{0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
	{0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
	{0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}, // D
	{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
	{0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
	{0x0E,0x11,0x10,0x17,0x11,0x11,0x0E}, // G
	{0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
	{0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // I
	{0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // J
	{0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
	{0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
	{0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // M
	{0x11,0x19,0x15,0x13,0x11,0x11,0x11}, // N
	{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
	{0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
	{0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
	{0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
	{0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}, // S
	{0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
	{0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
	{0x11,0x11,0x11,0x11,0x0A,0x0A,0x04}, // V
	{0x11,0x11,0x11,0x15,0x15,0x1B,0x11}, // W
	{0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
	{0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // Y
	{0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // Z
	// '[' (91)
	{0x0E,0x08,0x08,0x08,0x08,0x08,0x0E},
	// '\\' (92)
	{0x10,0x10,0x08,0x04,0x02,0x01,0x01},
	// ']' (93)
	{0x0E,0x02,0x02,0x02,0x02,0x02,0x0E},
	// '^' (94)
	{0x04,0x0A,0x11,0x00,0x00,0x00,0x00},
	// '_' (95)
	{0x00,0x00,0x00,0x00,0x00,0x00,0x1F},
	// '`' (96)
	{0x08,0x04,0x00,0x00,0x00,0x00,0x00},
	// a-z (97-122)
	{0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F}, // a
	{0x10,0x10,0x1E,0x11,0x11,0x11,0x1E}, // b
	{0x00,0x00,0x0E,0x11,0x10,0x11,0x0E}, // c
	{0x01,0x01,0x0F,0x11,0x11,0x11,0x0F}, // d
	{0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E}, // e
	{0x06,0x08,0x1E,0x08,0x08,0x08,0x08}, // f
	{0x00,0x00,0x0F,0x11,0x0F,0x01,0x0E}, // g
	{0x10,0x10,0x1E,0x11,0x11,0x11,0x11}, // h
	{0x04,0x00,0x0C,0x04,0x04,0x04,0x0E}, // i
	{0x02,0x00,0x06,0x02,0x02,0x12,0x0C}, // j
	{0x10,0x10,0x12,0x14,0x18,0x14,0x12}, // k
	{0x0C,0x04,0x04,0x04,0x04,0x04,0x0E}, // l
	{0x00,0x00,0x1A,0x15,0x15,0x15,0x15}, // m
	{0x00,0x00,0x1E,0x11,0x11,0x11,0x11}, // n
	{0x00,0x00,0x0E,0x11,0x11,0x11,0x0E}, // o
	{0x00,0x00,0x1E,0x11,0x1E,0x10,0x10}, // p
	{0x00,0x00,0x0F,0x11,0x0F,0x01,0x01}, // q
	{0x00,0x00,0x16,0x19,0x10,0x10,0x10}, // r
	{0x00,0x00,0x0F,0x10,0x0E,0x01,0x1E}, // s
	{0x08,0x08,0x1E,0x08,0x08,0x09,0x06}, // t
	{0x00,0x00,0x11,0x11,0x11,0x11,0x0F}, // u
	{0x00,0x00,0x11,0x11,0x11,0x0A,0x04}, // v
	{0x00,0x00,0x11,0x11,0x15,0x15,0x0A}, // w
	{0x00,0x00,0x11,0x0A,0x04,0x0A,0x11}, // x
	{0x00,0x00,0x11,0x11,0x0F,0x01,0x0E}, // y
	{0x00,0x00,0x1F,0x02,0x04,0x08,0x1F}, // z
};

// ── SDL drawing helpers ──

static void set_color(const Color &c) {
	SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
}

static void draw_rect(int x, int y, int w, int h, const Color &c) {
	set_color(c);
	SDL_Rect r = {x, y, w, h};
	SDL_RenderFillRect(g_renderer, &r);
}

static void draw_line(int x1, int y1, int x2, int y2, const Color &c) {
	set_color(c);
	SDL_RenderDrawLine(g_renderer, x1, y1, x2, y2);
}

static void draw_filled_circle(int cx, int cy, int radius, const Color &c) {
	set_color(c);
	for (int dy = -radius; dy <= radius; dy++) {
		int dx = (int)sqrtf((float)(radius * radius - dy * dy));
		SDL_RenderDrawLine(g_renderer, cx - dx, cy + dy, cx + dx, cy + dy);
	}
}

static void draw_circle_outline(int cx, int cy, int radius, const Color &c) {
	set_color(c);
	int x = radius, y = 0, err = 1 - radius;
	while (x >= y) {
		SDL_RenderDrawPoint(g_renderer, cx + x, cy + y);
		SDL_RenderDrawPoint(g_renderer, cx - x, cy + y);
		SDL_RenderDrawPoint(g_renderer, cx + x, cy - y);
		SDL_RenderDrawPoint(g_renderer, cx - x, cy - y);
		SDL_RenderDrawPoint(g_renderer, cx + y, cy + x);
		SDL_RenderDrawPoint(g_renderer, cx - y, cy + x);
		SDL_RenderDrawPoint(g_renderer, cx + y, cy - x);
		SDL_RenderDrawPoint(g_renderer, cx - y, cy - x);
		y++;
		if (err < 0) {
			err += 2 * y + 1;
		} else {
			x--;
			err += 2 * (y - x) + 1;
		}
	}
}

// Draw text using the 5x7 bitmap font, scaled by `scale`
static void draw_text(const char *text, int x, int y, int scale, const Color &c) {
	set_color(c);
	int ox = x;
	for (const char *p = text; *p; p++) {
		if (*p == '\n') { y += 8 * scale; x = ox; continue; }
		int ch = (unsigned char)*p;
		if (ch < 32 || ch > 122) ch = '?';
		const uint8_t *glyph = font5x7[ch - 32];
		for (int row = 0; row < 7; row++) {
			uint8_t bits = glyph[row];
			for (int col = 0; col < 5; col++) {
				if (bits & (0x10 >> col)) {
					SDL_Rect r = {x + col * scale, y + row * scale, scale, scale};
					SDL_RenderFillRect(g_renderer, &r);
				}
			}
		}
		x += 6 * scale;
	}
}

// Star points (hoshi) on a 19x19 board
static const int hoshi[][2] = {
	{3,3}, {3,9}, {3,15},
	{9,3}, {9,9}, {9,15},
	{15,3}, {15,9}, {15,15}
};

void gui_init() {
	SDL_Init(SDL_INIT_VIDEO);
	g_window = SDL_CreateWindow("Gomoku",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_W, WINDOW_H, 0);
	g_renderer = SDL_CreateRenderer(g_window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
	g_keys = SDL_GetKeyboardState(nullptr);
}

void gui_close() {
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
}

bool gui_should_close() {
	return false; // handled via events
}

// ── Event handling ──

void gui_poll_events(GameState &s, GuiState &g, bool &quit) {
	g_mouse_pressed = false;
	memset(g_key_just, 0, sizeof(g_key_just));

	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_QUIT) {
			quit = true;
			return;
		}
		if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
			g_mouse_pressed = true;
		}
		if (ev.type == SDL_KEYDOWN && !ev.key.repeat) {
			g_key_just[ev.key.keysym.scancode] = true;
		}
	}
	SDL_GetMouseState(&g_mouse_x, &g_mouse_y);

	// Keyboard shortcuts
	if (g_key_just[SDL_SCANCODE_N]) {
		init_game(s);
		g.undo_stack.clear();
		g.redo_stack.clear();
		g.ai_thinking = false;
		g.ai_time = 0;
		g.hint_move = -1;
		tt_clear();
	}
	if (g_key_just[SDL_SCANCODE_M]) {
		g.mode = (g.mode == MODE_PVA) ? MODE_PVP : MODE_PVA;
		init_game(s);
		g.undo_stack.clear();
		g.redo_stack.clear();
		g.ai_thinking = false;
		g.ai_time = 0;
		g.hint_move = -1;
		tt_clear();
	}
	if (g_key_just[SDL_SCANCODE_T]) {
		g.theme_idx = (g.theme_idx + 1) % 2;
	}
	if (g_key_just[SDL_SCANCODE_H]) {
		g.show_hint = !g.show_hint;
		if (g.show_hint && !s.game_over) {
			g.hint_move = ai_suggest_move(s, s.current);
		}
	}
	if (g_key_just[SDL_SCANCODE_D]) {
		g.show_heatmap = !g.show_heatmap;
	}
	// Undo (Z)
	if (g_key_just[SDL_SCANCODE_Z] && !g.undo_stack.empty() && !g.ai_thinking) {
		g.redo_stack.push_back(make_snapshot(s));
		restore_snapshot(s, g.undo_stack.back());
		g.undo_stack.pop_back();
		if (g.mode == MODE_PVA && !g.undo_stack.empty()) {
			g.redo_stack.push_back(make_snapshot(s));
			restore_snapshot(s, g.undo_stack.back());
			g.undo_stack.pop_back();
		}
		g.hint_move = -1;
	}
	// Redo (Y)
	if (g_key_just[SDL_SCANCODE_Y] && !g.redo_stack.empty() && !g.ai_thinking) {
		g.undo_stack.push_back(make_snapshot(s));
		restore_snapshot(s, g.redo_stack.back());
		g.redo_stack.pop_back();
		if (g.mode == MODE_PVA && !g.redo_stack.empty()) {
			g.undo_stack.push_back(make_snapshot(s));
			restore_snapshot(s, g.redo_stack.back());
			g.redo_stack.pop_back();
		}
		g.hint_move = -1;
	}
}

// ── Board drawing ──

static void draw_board(const GameState &s, const GuiState &g) {
	const Theme &t = themes[g.theme_idx];

	// Board background
	draw_rect(0, 0, BOARD_PX, BOARD_PX, t.board_bg);

	// Grid lines
	for (int i = 0; i < BSIZE; i++) {
		int x = board_x(i);
		int y = board_y(i);
		draw_line(x, board_y(0), x, board_y(BSIZE - 1), t.line_color);
		draw_line(board_x(0), y, board_x(BSIZE - 1), y, t.line_color);
	}

	// Star points
	for (int i = 0; i < 9; i++)
		draw_filled_circle(board_x(hoshi[i][1]), board_y(hoshi[i][0]), 4, t.line_color);

	// Stones
	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] == EMPTY) continue;
		int x = board_x(col_of(i));
		int y = board_y(row_of(i));
		Color c = (s.board[i] == STONE_BLACK) ? t.black_stone : t.white_stone;
		draw_filled_circle(x, y, STONE_RADIUS, c);
		if (s.board[i] == STONE_WHITE)
			draw_circle_outline(x, y, STONE_RADIUS, t.line_color);
	}

	// Last move indicator
	if (s.last_move >= 0) {
		int x = board_x(col_of(s.last_move));
		int y = board_y(row_of(s.last_move));
		draw_filled_circle(x, y, 5, t.highlight);
	}

	// Hover preview
	if (!s.game_over && !g.ai_thinking) {
		int mc = (int)roundf((g_mouse_x - BOARD_MARGIN) / (float)CELL_SIZE);
		int mr = (int)roundf((g_mouse_y - BOARD_MARGIN) / (float)CELL_SIZE);
		if (in_bounds(mr, mc) && s.board[idx(mr, mc)] == EMPTY) {
			Color preview = (s.current == STONE_BLACK) ? t.black_stone : t.white_stone;
			preview.a = 100;
			draw_filled_circle(board_x(mc), board_y(mr), STONE_RADIUS, preview);
		}
	}

	// Hint overlay
	if (g.show_hint && g.hint_move >= 0 && !s.game_over) {
		int x = board_x(col_of(g.hint_move));
		int y = board_y(row_of(g.hint_move));
		Color green = {0, 200, 0, 100};
		draw_filled_circle(x, y, STONE_RADIUS - 2, green);
		Color white = {255, 255, 255, 255};
		draw_text("H", x - 3, y - 5, 2, white);
	}
}

static void draw_heatmap(const GameState &s, const GuiState &g) {
	if (!g.show_heatmap) return;
	const Theme &t = themes[g.theme_idx];

	for (int i = 0; i < BCELLS; i++) {
		if (s.board[i] != EMPTY) continue;

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

		float intensity = (float)score / 100000.0f;
		if (intensity > 1.0f) intensity = 1.0f;
		if (intensity < 0.0f) intensity = 0.0f;
		Color heat = {
			(uint8_t)(255 * (1.0f - intensity)),
			(uint8_t)(255 * intensity),
			0, 120
		};
		draw_rect(x - CELL_SIZE/2, y - CELL_SIZE/2, CELL_SIZE, CELL_SIZE, heat);

		char buf[16];
		snprintf(buf, sizeof(buf), "%d", score / 1000);
		draw_text(buf, x - 6, y - 4, 1, t.text_color);
	}
}

static void draw_panel(const GameState &s, const GuiState &g) {
	const Theme &t = themes[g.theme_idx];
	int px = BOARD_PX;
	int pw = PANEL_WIDTH;

	draw_rect(px, 0, pw, WINDOW_H, t.panel_bg);

	int y = 20;
	draw_text("GOMOKU", px + 20, y, 4, t.text_color);
	y += 50;

	// Turn
	const char *turn_str = s.game_over
		? (s.winner == EMPTY ? "Draw!" : (s.winner == STONE_BLACK ? "Black wins!" : "White wins!"))
		: (s.current == STONE_BLACK ? "Turn: Black" : "Turn: White");
	draw_text(turn_str, px + 20, y, 2, t.text_color);
	y += 30;

	// AI thinking
	if (g.ai_thinking) {
		draw_text("AI thinking...", px + 20, y, 2, t.highlight);
	} else if (g.ai_time > 0) {
		char buf[64];
		snprintf(buf, sizeof(buf), "AI time: %.3fs", g.ai_time);
		draw_text(buf, px + 20, y, 2, t.text_color);
	}
	y += 30;

	// Captures
	char cap_buf[64];
	snprintf(cap_buf, sizeof(cap_buf), "Black captures: %d", s.captures[0]);
	draw_text(cap_buf, px + 20, y, 2, t.text_color);
	y += 22;
	snprintf(cap_buf, sizeof(cap_buf), "White captures: %d", s.captures[1]);
	draw_text(cap_buf, px + 20, y, 2, t.text_color);
	y += 30;

	// Move count
	char mc_buf[32];
	snprintf(mc_buf, sizeof(mc_buf), "Moves: %d", s.move_count);
	draw_text(mc_buf, px + 20, y, 2, t.text_color);
	y += 35;

	// Mode
	const char *mode_str = (g.mode == MODE_PVA) ? "Mode: vs AI" : "Mode: vs Human";
	draw_text(mode_str, px + 20, y, 2, t.text_color);
	y += 35;

	// Buttons
	int btn_w = pw - 40;
	int btn_h = 30;

	draw_rect(px + 20, y, btn_w, btn_h, t.btn_bg);
	draw_text("New Game (N)", px + 30, y + 8, 2, t.btn_text);
	y += btn_h + 8;

	draw_rect(px + 20, y, btn_w, btn_h, t.btn_bg);
	const char *toggle_str = (g.mode == MODE_PVA) ? "Switch to PvP (M)" : "Switch to PvAI (M)";
	draw_text(toggle_str, px + 30, y + 8, 2, t.btn_text);
	y += btn_h + 8;

	draw_rect(px + 20, y, btn_w, btn_h, t.btn_bg);
	draw_text("Undo (Z)", px + 30, y + 8, 2, t.btn_text);
	y += btn_h + 8;

	draw_rect(px + 20, y, btn_w, btn_h, t.btn_bg);
	draw_text("Redo (Y)", px + 30, y + 8, 2, t.btn_text);
	y += btn_h + 20;

	// Shortcuts
	draw_text("Shortcuts:", px + 20, y, 2, t.text_color);
	y += 20;
	draw_text("H - Hint", px + 20, y, 2, t.text_color);
	y += 16;
	draw_text("T - Theme", px + 20, y, 2, t.text_color);
	y += 16;
	draw_text("D - Heatmap", px + 20, y, 2, t.text_color);
	y += 16;
	draw_text("N - New Game", px + 20, y, 2, t.text_color);
	y += 16;
	draw_text("M - Toggle Mode", px + 20, y, 2, t.text_color);
}

void gui_draw(const GameState &s, const GuiState &g) {
	const Theme &t = themes[g.theme_idx];
	set_color(t.panel_bg);
	SDL_RenderClear(g_renderer);
	draw_board(s, g);
	draw_heatmap(s, g);
	draw_panel(s, g);
	SDL_RenderPresent(g_renderer);
}

int gui_handle_input(GameState &s, GuiState &g) {
	if (g_mouse_pressed && !s.game_over && !g.ai_thinking) {
		int mc = (int)roundf((g_mouse_x - BOARD_MARGIN) / (float)CELL_SIZE);
		int mr = (int)roundf((g_mouse_y - BOARD_MARGIN) / (float)CELL_SIZE);
		if (in_bounds(mr, mc)) {
			int pos = idx(mr, mc);
			if (s.board[pos] == EMPTY && is_legal_move(s, pos))
				return pos;
		}
	}
	return -1;
}
