#include <pebble.h>

// Constants

#define Player_Count 4
#define Player_TopLeft 0
#define Player_TopRight 1
#define Player_BottomLeft 2
#define Player_BottomRight 3
#define Player_User Player_BottomLeft
#define Player_PosRange 100
#define Player_AngleGap 10

#define Canvas_Width PBL_DISPLAY_WIDTH
#define Canvas_Height PBL_DISPLAY_HEIGHT

#if defined(PBL_PLATFORM_EMERY)
#define Base_Size 8
#define Base_Inner_Size 4
#elif defined(PBL_PLATFORM_CHALK)
#define Base_Size 8
#define Base_Inner_Size 6
#else
#define Base_Size 6
#define Base_Inner_Size 4
#endif

// Structs

struct FPoint {
    float x;
    float y;
};

// App Layers
Window *game_window;
Layer *game_canvas;

// Game Logic
GColor base_colors[4];

float player_pos[Player_Count]; // 0.0 -> 100.0
const float player_speed = 2.5;
GColor player_colors[Player_Count];
bool player_alive[Player_Count];

#if defined(PBL_PLATFORM_EMERY)
uint8_t player_damage[Player_Count][8] = {
    { // TopLeft
        0b11110000, // 0
        0b11110000, // 1
        0b11110000, // 2
        0b11110000, // 3
        0b11111111, // 4
        0b11111111, // 5
        0b11111111, // 6
        0b11111111  // 7
    }, { // TopRight
        0b00001111, // 0
        0b00001111, // 1
        0b00001111, // 2
        0b00001111, // 3
        0b11111111, // 4
        0b11111111, // 5
        0b11111111, // 6
        0b11111111  // 7
    }, { // BottomLeft
        0b11111111, // 0
        0b11111111, // 1
        0b11111111, // 2
        0b11111111, // 3
        0b11110000, // 4
        0b11110000, // 5
        0b11110000, // 6
        0b11110000  // 7
    }, { // BottomRight
        0b11111111, // 0
        0b11111111, // 1
        0b11111111, // 2
        0b11111111, // 3
        0b00001111, // 4
        0b00001111, // 5
        0b00001111, // 6
        0b00001111  // 7
    }
};
#elif defined(PBL_PLATFORM_CHALK)
uint8_t player_damage[Player_Count][8] = {
    { // TopLeft
        0b11000000, // 0
        0b11000000, // 1
        0b11000000, // 2
        0b11000000, // 3
        0b11000000, // 4
        0b11000000, // 5
        0b11111111, // 6
        0b11111111  // 7
    }, { // TopRight
        0b00000011, // 0
        0b00000011, // 1
        0b00000011, // 2
        0b00000011, // 3
        0b00000011, // 4
        0b00000011, // 5
        0b11111111, // 6
        0b11111111  // 7
    }, { // BottomLeft
        0b11111111, // 0
        0b11111111, // 1
        0b11000000, // 2
        0b11000000, // 3
        0b11000000, // 4
        0b11000000, // 5
        0b11000000, // 6
        0b11000000  // 7
    }, { // BottomRight
        0b11111111, // 0
        0b11111111, // 1
        0b00000011, // 2
        0b00000011, // 3
        0b00000011, // 4
        0b00000011, // 5
        0b00000011, // 6
        0b00000011  // 7
    }
};
#else
uint8_t player_damage[Player_Count][8] = {
    { // TopLeft
        0b00110000, // 0
        0b00110000, // 1
        0b00110000, // 2
        0b00110000, // 3
        0b00111111, // 4
        0b00111111, // 5
        0b00000000, // 6
        0b00000000  // 7
    }, { // TopRight
        0b00001100, // 0
        0b00001100, // 1
        0b00001100, // 2
        0b00001100, // 3
        0b11111100, // 4
        0b11111100, // 5
        0b00000000, // 6
        0b00000000  // 7
    }, { // BottomLeft
        0b00000000, // 0
        0b00000000, // 1
        0b00111111, // 2
        0b00111111, // 3
        0b00110000, // 4
        0b00110000, // 5
        0b00110000, // 6
        0b00110000  // 7
    }, { // BottomRight
        0b00000000, // 0
        0b00000000, // 1
        0b11111100, // 2
        0b11111100, // 3
        0b00001100, // 4
        0b00001100, // 5
        0b00001100, // 6
        0b00001100  // 7
    }
};
#endif

struct FPoint ball_pos;
struct FPoint ball_speed;
const float ball_speed_max = 2.0;
const uint16_t ball_radius = 2;
GColor ball_color;

// Button Logic
bool button_sel = false;
bool button_up = false;
bool button_down = false;

// Sprites

const uint16_t sprite_knight[16] = {
    0b0000000000000000, // 0
    0b0000001111000000, // 1
    0b0000001111000000, // 2
    0b0011111111111100, // 3
    0b0011111111111100, // 4
    0b1111001111001111, // 5
    0b1111001111001111, // 6
    0b1111000000001111, // 7
    0b1111000000001111, // 8
    0b1111110000111111, // 9
    0b1111110000111111, // 10
    0b1111110000111111, // 11
    0b1111110000111111, // 12
    0b0011110000111100, // 13
    0b0011110000111100, // 14
    0b0000000000000000  // 15
};
const uint16_t sprite_castle[16] = {
    0b0000000000000000, // 0
    0b1111001111001111, // 1
    0b1111001111001111, // 2
    0b1111111111111111, // 3
    0b1111111111111111, // 4
    0b1111111111111111, // 5
    0b1111111111111111, // 6
    0b0000000000000000, // 7
    0b0000000000000000, // 8
    0b0000111111110000, // 9
    0b0000111111110000, // 10
    0b0000111111110000, // 11
    0b0000111111110000, // 12
    0b0000111111110000, // 13
    0b0000111111110000, // 14
    0b0000000000000000  // 15
};

#if defined(PBL_PLATFORM_EMERY)
const uint16_t sprite_player[8][16] = { // Rotation 0 -> 359 = 0 -> 7
    {    // 0 = right
        0b0011111111000000, // 0
        0b0011111111000000, // 1
        0b0000111111110000, // 2
        0b0000111111110000, // 3
        0b0000001111111100, // 4
        0b0000001111111100, // 5
        0b0000001111111100, // 6
        0b0000001111111100, // 7
        0b0000001111111100, // 8
        0b0000001111111100, // 9
        0b0000001111111100, // 10
        0b0000001111111100, // 11
        0b0000111111110000, // 12
        0b0000111111110000, // 13
        0b0011111111000000, // 14
        0b0011111111000000  // 15
    }, { // 1 = diagonal up right
        0b0000000000000000, // 0
        0b0000000000000000, // 1
        0b0011111111110000, // 2
        0b0011111111110000, // 3
        0b1111111111111100, // 4
        0b1111111111111100, // 5
        0b0011111111111100, // 6
        0b0011111111111100, // 7
        0b0000001111111100, // 8
        0b0000001111111100, // 9
        0b0000000011111100, // 10
        0b0000000011111100, // 11
        0b0000000011111100, // 12
        0b0000000011111100, // 13
        0b0000000000110000, // 14
        0b0000000000110000  // 15
    }, { // 2 = up
        0b0000000000000000, // 0
        0b0000000000000000, // 1
        0b0000111111110000, // 2
        0b0000111111110000, // 3
        0b0011111111111100, // 4
        0b0011111111111100, // 5
        0b1111111111111111, // 6
        0b1111111111111111, // 7
        0b1111111111111111, // 8
        0b1111111111111111, // 9
        0b1111000000001111, // 10
        0b1111000000001111, // 11
        0b1100000000000011, // 12
        0b1100000000000011, // 13
        0b0000000000000000, // 14
        0b0000000000000000  // 15
    }, { // 3 = diagonal up left
        0b0000000000000000, // 0
        0b0000000000000000, // 1
        0b0000111111111100, // 2
        0b0000111111111100, // 3
        0b0011111111111111, // 4
        0b0011111111111111, // 5
        0b0011111111111100, // 6
        0b0011111111111100, // 7
        0b0011111111000000, // 8
        0b0011111111000000, // 9
        0b0011111100000000, // 10
        0b0011111100000000, // 11
        0b0011111100000000, // 12
        0b0011111100000000, // 13
        0b0000110000000000, // 14
        0b0000110000000000  // 15
    }, { // 4 = left
        0b0000001111111100, // 0
        0b0000001111111100, // 1
        0b0000111111110000, // 2
        0b0000111111110000, // 3
        0b0011111111000000, // 4
        0b0011111111000000, // 5
        0b0011111111000000, // 6
        0b0011111111000000, // 7
        0b0011111111000000, // 8
        0b0011111111000000, // 9
        0b0011111111000000, // 10
        0b0011111111000000, // 11
        0b0000111111110000, // 12
        0b0000111111110000, // 13
        0b0000001111111100, // 14
        0b0000001111111100  // 15
    }, { // 5 = diagonal down left
        0b0000110000000000, // 0
        0b0000110000000000, // 1
        0b0011111100000000, // 2
        0b0011111100000000, // 3
        0b0011111100000000, // 4
        0b0011111100000000, // 5
        0b0011111111000000, // 6
        0b0011111111000000, // 7
        0b0011111111111100, // 8
        0b0011111111111100, // 9
        0b0011111111111111, // 10
        0b0011111111111111, // 11
        0b0000111111111100, // 12
        0b0000111111111100, // 13
        0b0000000000000000, // 14
        0b0000000000000000  // 15
    }, { // 6 = down
        0b0000000000000000, // 0
        0b0000000000000000, // 1
        0b1100000000000011, // 2
        0b1100000000000011, // 3
        0b1111000000001111, // 4
        0b1111000000001111, // 5
        0b1111111111111111, // 6
        0b1111111111111111, // 7
        0b1111111111111111, // 8
        0b1111111111111111, // 9
        0b0011111111111100, // 10
        0b0011111111111100, // 11
        0b0000111111110000, // 12
        0b0000111111110000, // 13
        0b0000000000000000, // 14
        0b0000000000000000  // 15
    }, { // 7 = diagonal down right
        0b0000000000110000, // 0
        0b0000000000110000, // 1
        0b0000000011111100, // 2
        0b0000000011111100, // 3
        0b0000000011111100, // 4
        0b0000000011111100, // 5
        0b0000001111111100, // 6
        0b0000001111111100, // 7
        0b0011111111111100, // 8
        0b0011111111111100, // 9
        0b1111111111111100, // 10
        0b1111111111111100, // 11
        0b0011111111110000, // 12
        0b0011111111110000, // 13
        0b0000000000000000, // 14
        0b0000000000000000  // 15
    }
};
#else
const uint16_t sprite_player[8][8] = { // Rotation 0 -> 359 = 0 -> 7
    {    // 0 = right
        0b01111000, // 0
        0b00111100, // 1
        0b00011110, // 2
        0b00011110, // 3
        0b00011110, // 4
        0b00011110, // 5
        0b00111100, // 6
        0b01111000  // 7
    }, { // 1 = diagonal up right
        0b00000000, // 0
        0b01111100, // 1
        0b11111110, // 2
        0b01111110, // 3
        0b00011110, // 4
        0b00001110, // 5
        0b00001110, // 6
        0b00000100  // 7
    }, { // 2 = up
        0b00000000, // 0
        0b00111100, // 1
        0b01111110, // 2
        0b11111111, // 3
        0b11111111, // 4
        0b11000011, // 5
        0b10000001, // 6
        0b00000000  // 7
    }, { // 3 = diagonal up left
        0b00000000, // 0
        0b00111110, // 1
        0b01111111, // 2
        0b01111110, // 3
        0b01111000, // 4
        0b01110000, // 5
        0b01110000, // 6
        0b00100000  // 7
    }, { // 4 = left
        0b00011110, // 0
        0b00111100, // 1
        0b01111000, // 2
        0b01111000, // 3
        0b01111000, // 4
        0b01111000, // 5
        0b00111100, // 6
        0b00011110  // 7
    }, { // 5 = diagonal down left
        0b00100000, // 0
        0b01110000, // 1
        0b01110000, // 2
        0b01111000, // 3
        0b01111110, // 4
        0b01111111, // 5
        0b00111110, // 6
        0b00000000  // 7
    }, { // 6 = down
        0b00000000, // 0
        0b10000001, // 1
        0b11000011, // 2
        0b11111111, // 3
        0b11111111, // 4
        0b01111110, // 5
        0b00111100, // 6
        0b00000000  // 7
    }, { // 7 = diagonal down right
        0b00000100, // 0
        0b00001110, // 1
        0b00001110, // 2
        0b00011110, // 3
        0b01111110, // 4
        0b11111110, // 5
        0b01111100, // 6
        0b00000000  // 7
    }
};
#endif
GRect player_size;

const uint16_t sprite_block[8] = {
    0b1111111111111111, // 0
    0b1111111111111111, // 1
    0b1111111100000000, // 2
    0b1111111111111111, // 3
    0b1111111111111111, // 4
    0b1111111111111111, // 5
    0b1111111111111111, // 6
    0b0000000011111111  // 7
};

// Initialization
void game_init(void);
void game_deinit(void);
static void game_timer(void *context);

// Update Handler
static void game_update(Layer *layer, GContext *ctx);

// Logic
void game_logic_init(void);
static void game_logic(void);
static void game_logic_player(uint8_t player);
static void game_logic_ball(void);

// Drawing
void game_draw_init(void);
static void game_draw(Layer *layer, GContext *ctx);
static void game_draw_base(Layer *layer, GContext *ctx, uint8_t player);
static void game_draw_player(Layer *layer, GContext *ctx, uint8_t player);
static void game_draw_ball(Layer *layer, GContext *ctx);
static void game_draw_bitmap(Layer *layer, GContext *ctx, uint16_t pos_x, uint16_t pos_y, const uint16_t bitmap[], GRect bounds, GColor color);

// Game Functions
static GPoint get_player_position(uint8_t player);

// Button Handling
void game_button_config(Window *window);
void game_button_sel_click(ClickRecognizerRef recognizer, void *context);
void game_button_sel_release(ClickRecognizerRef recognizer, void *context);
void game_button_up_click(ClickRecognizerRef recognizer, void *context);
void game_button_up_release(ClickRecognizerRef recognizer, void *context);
void game_button_down_click(ClickRecognizerRef recognizer, void *context);
void game_button_down_release(ClickRecognizerRef recognizer, void *context);

// Main Function Call

int main(void) {
    game_init();
    app_event_loop();
    game_deinit();
}

// Initialization

void game_init(void) {
    // Setup Window
    game_window = window_create();
    window_set_click_config_provider(game_window, (ClickConfigProvider)game_button_config);

    // Setup Layer
    game_canvas = layer_create(GRect(0, 0, Canvas_Width, Canvas_Height));

    window_stack_push(game_window, true);
    Layer* root_layer = window_get_root_layer(game_window);
    layer_add_child(root_layer, game_canvas);

    // Setup Color Scheme
    base_colors[0] = GColorWindsorTan;
    base_colors[1] = GColorOrange;
    base_colors[2] = GColorChromeYellow;
    base_colors[3] = GColorRajah;

    player_colors[0] = GColorPurple;
    player_colors[1] = GColorRed;
    player_colors[2] = GColorBlueMoon;
    player_colors[3] = GColorMintGreen;

    // Setup Game State

    player_alive[0] = player_alive[1] = player_alive[2] = player_alive[3] = true;

    #if defined(PBL_PLATFORM_EMERY)
    player_size = GRect(0, 0, 16, 16);
    #else
    player_size = GRect(0, 0, 8, 8);
    #endif

    // Setup Game
    game_logic_init();
    game_draw_init();

    // Start Game Timers
    layer_set_update_proc(game_canvas, game_update);
    app_timer_register(34, game_timer, NULL);
}

void game_deinit(void) {
    layer_destroy(game_canvas);
    window_destroy(game_window);
}

static void game_timer(void *context) {
    layer_mark_dirty(game_canvas);
    app_timer_register(34, game_timer, NULL);
}

// Game Update

static void game_update(Layer *layer, GContext *ctx) {
    game_logic();
    game_draw(layer, ctx);
}

// Game Logic

void game_logic_init(void) {
    ball_pos.x = Canvas_Width >> 1;
    ball_pos.y = Canvas_Height >> 1;
    float angle = rand() % 360;
    ball_speed.x = ball_speed_max * cos_lookup(DEG_TO_TRIGANGLE(angle)) / TRIG_MAX_RATIO;
    ball_speed.y = ball_speed_max * sin_lookup(DEG_TO_TRIGANGLE(angle)) / TRIG_MAX_RATIO;

    player_pos[0] = player_pos[1] = player_pos[2] = player_pos[3] = 50.0;
}

static void game_logic(void) {
    game_logic_player(Player_TopLeft);
    game_logic_player(Player_TopRight);
    game_logic_player(Player_BottomLeft);
    game_logic_player(Player_BottomRight);

    game_logic_ball();
}

static void game_logic_player(uint8_t player) {
    // Player Control
    if (player == Player_User) {
        if (button_up == true) {
            player_pos[player] -= player_speed;
        }
        if (button_down == true) {
            player_pos[player] += player_speed;
        }
    } else {
        // TODO: Simple AI
    }

    if (player_pos[player] > Player_PosRange) {
        player_pos[player] = Player_PosRange;
    } else if (player_pos[player] < 0) {
        player_pos[player] = 0;
    }
}

static void game_logic_ball(void) {
    ball_pos.x += ball_speed.x;
    ball_pos.y += ball_speed.y;

    // Wall Reflection
    if (ball_pos.x <= 0 || ball_pos.x >= Canvas_Width) {
        ball_speed.x = -ball_speed.x;
        return;
    } else if (ball_pos.y <= 0 || ball_pos.y >= Canvas_Height) {
        ball_speed.y = -ball_speed.y;
        return;
    }

    // Base Reflection and Damage
    GPoint rel_pos = GPoint(ball_pos.x, ball_pos.y);

    int8_t horizontal = 0; // 0 = not found, -1 = left, 1 = right
    int8_t vertical = 0; // 0 = not found, -1 = up, 1 = down

    if (ball_pos.x < Base_Size << 3) {
        rel_pos.x = (uint16_t)ball_pos.x >> 3;
        horizontal = -1;
    } else if (ball_pos.x > Canvas_Width - (Base_Size << 3)) {
        rel_pos.x = ((uint16_t)ball_pos.x - (Canvas_Width - (Base_Size << 3))) >> 3;
        horizontal = 1;
    }
    if (ball_pos.y < Base_Size << 3) {
        rel_pos.y = (uint16_t)ball_pos.y >> 3;
        vertical = -1;
    } else if (ball_pos.y > Canvas_Height - (Base_Size << 3)) {
        rel_pos.y = ((uint16_t)ball_pos.y - (Canvas_Height - (Base_Size << 3))) >> 3;
        vertical = 1;
    }

    if (horizontal != 0 && vertical != 0) {
        // Determine Player
        uint8_t player = 0;
        if (horizontal < 0 && vertical < 0) {
            player = Player_TopLeft;
        } else if (horizontal > 0 && vertical < 0) {
            player = Player_TopRight;
        } else if (horizontal < 0 && vertical > 0) {
            player = Player_BottomLeft;
        } else if (horizontal > 0 && vertical > 0) {
            player = Player_BottomRight;
        }

        if (((horizontal < 0 && rel_pos.x < Base_Inner_Size) || (horizontal > 0 && rel_pos.x > Base_Size - Base_Inner_Size)) && ((vertical < 0 && rel_pos.y < Base_Inner_Size) || (vertical > 0 && rel_pos.y > Base_Size - Base_Inner_Size))) { // If inside player's inner base
            // Set player as dead
            player_alive[player] = false;

            // Reset ball
            ball_pos.x = Canvas_Width >> 1;
            ball_pos.y = Canvas_Height >> 1;
            float angle = rand() % 360;
            ball_speed.x = ball_speed_max * cos_lookup(DEG_TO_TRIGANGLE(angle)) / TRIG_MAX_RATIO;
            ball_speed.y = ball_speed_max * sin_lookup(DEG_TO_TRIGANGLE(angle)) / TRIG_MAX_RATIO;

            // TODO: Death animation
        } else if ((player_damage[player][rel_pos.y]  >> rel_pos.x) & 1) { // If hit
            player_damage[player][rel_pos.y] = player_damage[player][rel_pos.y] & ~(1 << rel_pos.x); // Deal damage (set as 0)

            // Reflect
            if ((horizontal > 0 && rel_pos.x < Base_Size >> 1) || (horizontal < 0 && rel_pos.x > Base_Size >> 1)) {
                ball_speed.x = -ball_speed.x;
            } else if ((vertical > 0 && rel_pos.y < Base_Size >> 1) || (vertical < 0 && rel_pos.y > Base_Size >> 1)) {
                ball_speed.y = -ball_speed.y;
            }
            // TODO: Better reflection. Gets in loop every now and then.
        }
        return;
    }

    // Paddle Reflection
    for (uint8_t i = 0; i < Player_Count; i++) {
        GPoint p_pos = get_player_position(i);
        if (ball_pos.x > p_pos.x && ball_pos.x < p_pos.x + player_size.size.w && ball_pos.y > p_pos.y && ball_pos.y < p_pos.y + player_size.size.h) { // Ball is inside player
            switch (i) {
                case Player_TopLeft:
                case Player_BottomLeft:
                    if (player_pos[i] < (Player_PosRange + Player_AngleGap) >> 1) {
                        ball_speed.y = -ball_speed.y;
                    }
                    if (player_pos[i] > (Player_PosRange - Player_AngleGap) >> 1) {
                        ball_speed.x = -ball_speed.x;
                    }
                    break;
                case Player_TopRight:
                case Player_BottomRight:
                    if (player_pos[i] < (Player_PosRange + Player_AngleGap) >> 1) {
                        ball_speed.x = -ball_speed.x;
                    }
                    if (player_pos[i] > (Player_PosRange - Player_AngleGap) >> 1) {
                        ball_speed.y = -ball_speed.y;
                    }
                    break;
            }
            break;
        }
    }
    // TODO: Better reflection to avoid infinite loops
}

// Game Drawing

void game_draw_init(void) {
    ball_color = GColorOrange;
}

static void game_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, Canvas_Width, Canvas_Height), 0, GCornersAll);

    game_draw_base(layer, ctx, Player_TopLeft);
    game_draw_base(layer, ctx, Player_TopRight);
    game_draw_base(layer, ctx, Player_BottomLeft);
    game_draw_base(layer, ctx, Player_BottomRight);

    game_draw_player(layer, ctx, Player_TopLeft);
    game_draw_player(layer, ctx, Player_TopRight);
    game_draw_player(layer, ctx, Player_BottomLeft);
    game_draw_player(layer, ctx, Player_BottomRight);

    game_draw_ball(layer, ctx);
}

static void game_draw_base(Layer *layer, GContext *ctx, uint8_t player) {
    uint16_t offset_x = 0;
    uint16_t offset_y = 0;
    switch (player) {
        case Player_TopRight:
        case Player_BottomRight:
            offset_x = Canvas_Width - (Base_Size << 3);
            break;
    }
    switch (player) {
        case Player_BottomLeft:
        case Player_BottomRight:
            offset_y = Canvas_Height - (Base_Size << 3);
            break;
    }

    GColor color = GColorWhite;

    uint8_t x, y;
    for (y = 0; y < Base_Size; y++) {

        #if defined(PBL_COLOR)
        switch (player) {
            case Player_TopLeft:
            case Player_TopRight:
                color = base_colors[y >> 1];
                break;
            case Player_BottomLeft:
            case Player_BottomRight:
                color = base_colors[3 - (y >> 1)];
                break;
        }
        #endif

        for (x = 0; x < Base_Size; x++) {
            if ((player_damage[player][y] & (1 << x)) >> x == 1) {
                game_draw_bitmap(layer, ctx, offset_x + x * 8, offset_y + y * 8, sprite_block, GRect((1 - (x % 2)) * 8, 0, 8, 8), color);
            }
        }
    }

    #if defined(PBL_COLOR)
    color = player_colors[player];
    #endif

    #if defined(PBL_PLATFORM_CHALK)
    x = 3;
    y = 3;
    #else
    switch (player) {
        case Player_TopLeft:
        case Player_BottomLeft:
            x = 1;
            break;
        case Player_TopRight:
        case Player_BottomRight:
            x = 5;
    }
    switch (player) {
        case Player_TopLeft:
        case Player_TopRight:
            y = 1;
            break;
        case Player_BottomLeft:
        case Player_BottomRight:
            y = 5;
            break;
    }
    #endif

    // Draw characters [16x16]
    if (player_alive[player] == true) {
        if (player == Player_User) { // Knight
            game_draw_bitmap(layer, ctx, offset_x + x * 8, offset_y + y * 8, sprite_knight, GRect(0, 0, 16, 16), color);
        } else { // Castle
            game_draw_bitmap(layer, ctx, offset_x + x * 8, offset_y + y * 8, sprite_castle, GRect(0, 0, 16, 16), color);
        }
    }
}

static void game_draw_player(Layer *layer, GContext *ctx, uint8_t player) {
    if (player_alive[player] == false) return;

    GPoint sprite_pos = get_player_position(player);

    uint8_t sprite = 0;
    if (player_pos[player] >= (Player_PosRange - Player_AngleGap) / 2 && player_pos[player] <= (Player_PosRange + Player_AngleGap) / 2) {
        switch (player) {
            case Player_TopLeft:
                sprite = 5;
                break;
            case Player_TopRight:
                sprite = 7;
                break;
            case Player_BottomLeft:
                sprite = 3;
                break;
            case Player_BottomRight:
                sprite = 1;
                break;
        }
    } else if (player_pos[player] < Player_PosRange / 2) {
        switch (player) {
            case Player_TopLeft:
                sprite = 6;
                break;
            case Player_TopRight:
                sprite = 0;
                break;
            case Player_BottomLeft:
                sprite = 2;
                break;
            case Player_BottomRight:
                sprite = 0;
                break;
        }
    } else {
        switch (player) {
            case Player_TopLeft:
                sprite = 4;
                break;
            case Player_TopRight:
                sprite = 6;
                break;
            case Player_BottomLeft:
                sprite = 4;
                break;
            case Player_BottomRight:
                sprite = 2;
                break;
        }
    }

    game_draw_bitmap(layer, ctx, sprite_pos.x, sprite_pos.y, sprite_player[sprite], player_size, player_colors[player]);
}

static void game_draw_ball(Layer *layer, GContext *ctx) {
    #if defined(PBL_COLOR)
    graphics_context_set_fill_color(ctx, ball_color);
    #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    #endif
    graphics_fill_circle(ctx, GPoint((uint16_t)ball_pos.x, (uint16_t)ball_pos.y), ball_radius);
}

static void game_draw_bitmap(Layer *layer, GContext *ctx, uint16_t pos_x, uint16_t pos_y, const uint16_t bitmap[], GRect bounds, GColor color) {
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    GRect fb_bounds = layer_get_bounds(layer);
    uint16_t i, j, x, y;

    for (i = bounds.origin.y, y = pos_y; i < bounds.size.h + bounds.origin.y; i++, y++) {
        if (y < fb_bounds.origin.y) continue;
        if (y >= fb_bounds.size.h + fb_bounds.origin.y) break;

        GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);

        for (j = bounds.origin.x, x = pos_x; j < bounds.size.w + bounds.origin.x; j++, x++) {
            if (x < info.min_x) continue;
            if (x >= info.max_x) break;

            if ((bitmap[i] & (1 << j)) >> j == 1) {
                #if defined(PBL_COLOR)
                memset(&info.data[x], color.argb, 1);
                #else
                info.data[x / 8] ^= (1 ^ info.data[x / 8]) & ((1 << x) % 8);
                #endif
            }
        }
    }

    graphics_release_frame_buffer(ctx, fb);
}

// Game Functions

static GPoint get_player_position(uint8_t player) {
    uint16_t dist = (Base_Size + 1) << 3;

    GPoint offset = GPoint(0, 0);
    switch (player) {
        case Player_TopRight:
        case Player_BottomRight:
            offset.x = Canvas_Width - dist - player_size.size.w;
            break;
    }
    switch (player) {
        case Player_BottomLeft:
        case Player_BottomRight:
            offset.y = Canvas_Height - dist - player_size.size.h;
            break;
    }

    GPoint pos = GPoint(0, 0);

    switch (player) {
        case Player_TopLeft:
        case Player_BottomLeft:
            if (player_pos[player] >= Player_PosRange >> 1) {
                pos.x = dist;
            } else {
                pos.x = player_pos[player] / (Player_PosRange >> 1) * dist;
            }
            break;
        case Player_TopRight:
        case Player_BottomRight:
            if (player_pos[player] >= Player_PosRange >> 1) {
                pos.x = (player_pos[player] - (Player_PosRange >> 1)) / (Player_PosRange >> 1) * dist;
            }
            break;
    }

    switch (player) {
        case Player_TopLeft:
            if (player_pos[player] < Player_PosRange >> 1) {
                pos.y = dist;
            } else {
                pos.y = (Player_PosRange - player_pos[player]) / (Player_PosRange >> 1) * dist;
            }
            break;
        case Player_TopRight:
            if (player_pos[player] >= Player_PosRange >> 1) {
                pos.y = dist;
            } else {
                pos.y = player_pos[player] / (Player_PosRange >> 1) * dist;
            }
            break;
        case Player_BottomLeft:
            if (player_pos[player] >= Player_PosRange >> 1) {
                pos.y = (player_pos[player] - (Player_PosRange >> 1)) / (Player_PosRange >> 1) * dist;
            }
            break;
        case Player_BottomRight:
            if (player_pos[player] < Player_PosRange >> 1) {
                pos.y = ((Player_PosRange >> 1) - player_pos[player]) / (Player_PosRange >> 1) * dist;
            }
            break;
    }

    pos.x += offset.x;
    pos.y += offset.y;

    return pos;
}

// Button Logic

void game_button_config(Window *window) {
    window_raw_click_subscribe(BUTTON_ID_SELECT, game_button_sel_click, game_button_sel_release, NULL);
    window_raw_click_subscribe(BUTTON_ID_UP, game_button_up_click, game_button_up_release, NULL);
    window_raw_click_subscribe(BUTTON_ID_DOWN, game_button_down_click, game_button_down_release, NULL);
}

void game_button_sel_click(ClickRecognizerRef recognizer, void *context) {
    button_sel = true;
}
void game_button_sel_release(ClickRecognizerRef recognizer, void *context) {
    button_sel = false;
}

void game_button_up_click(ClickRecognizerRef recognizer, void *context) {
    button_up = true;
}
void game_button_up_release(ClickRecognizerRef recognizer, void *context) {
    button_up = false;
}

void game_button_down_click(ClickRecognizerRef recognizer, void *context) {
    button_down = true;
}
void game_button_down_release(ClickRecognizerRef recognizer, void *context) {
    button_down = false;
}
