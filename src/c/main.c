#include <pebble.h>

// App Layers
Window *game_window;
Layer *game_canvas;

// Game Logic
uint8_t player_pos[4];
float player_speed = 1.0;

const float aiReaction = 0.2;
const float aiError = 40;

GPoint ball_pos;
float ball_pos_x;
float ball_pos_y;
float ball_speed;
float ball_speed_cos;
float ball_speed_sin;
float ball_angle; // degrees 0 -> 360
const uint16_t ball_radius = 2;
const float ball_acceleration = -0.02;
const float ball_speed_min = 0.005;
GColor ball_color;

// Button Logic
bool button_sel = false;
bool button_up = false;
bool button_down = false;

// Sprites

const uint16_t sprite_knight[16] = {
    0b0000000000000000, // 0
    0b0000000000000000, // 1
    0b0000001111000000, // 2
    0b0000001111000000, // 3
    0b0011111111111100, // 4
    0b0011111111111100, // 5
    0b1111001111001111, // 6
    0b1111001111001111, // 7
    0b1111000000001111, // 8
    0b1111000000001111, // 9
    0b1111110000111111, // 10
    0b1111110000111111, // 11
    0b1111110000111111, // 12
    0b1111110000111111, // 13
    0b0011110000111100, // 14
    0b0011110000111100  // 15
};
const uint16_t sprite_castle[16] = {
    0b0000000000000000, // 0
    0b0000000000000000, // 1
    0b1111001111001111, // 2
    0b1111001111001111, // 3
    0b1111111111111111, // 4
    0b1111111111111111, // 5
    0b1111111111111111, // 6
    0b1111111111111111, // 7
    0b0000000000000000, // 8
    0b0000000000000000, // 9
    0b0000111111110000, // 10
    0b0000111111110000, // 11
    0b0000111111110000, // 12
    0b0000111111110000, // 13
    0b0000111111110000, // 14
    0b0000111111110000  // 15
};

// Initialization
void game_init(void);
void game_deinit(void);
static void game_timer(void *context);

// Logic + Drawing
static void game_update(Layer *layer, GContext *ctx);

void game_logic_init(void);
static void game_logic(void);
static void game_logic_ball(void);
static void game_logic_ball_update_angle(void);

void game_draw_init(void);
static void game_draw(Layer *layer, GContext *ctx);
static void game_draw_base(Layer *layer, GContext *ctx, int player);
static void game_draw_ball(Layer *layer, GContext *ctx);
static void game_draw_bitmap(Layer *layer, GContext *ctx, uint16_t pos_x, uint16_t pos_y, const uint16_t bitmap[], GRect bounds, GColor color);

#define Player_TopLeft 1
#define Player_TopRight 2
#define Player_BottomLeft 3
#define Player_BottomRight 4

#define Canvas_Width PBL_DISPLAY_WIDTH
#define Canvas_Height PBL_DISPLAY_HEIGHT

#define Base_Size Canvas_Width / 3
#define Base_Inner_Size Base_Size * 2 / 3
// Make sure these are integers

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
    game_window = window_create();
    window_set_click_config_provider(game_window, (ClickConfigProvider)game_button_config);

    game_canvas = layer_create(GRect(0, 0, Canvas_Width, Canvas_Height));

    window_stack_push(game_window, true);

    Layer* root_layer = window_get_root_layer(game_window);
    layer_add_child(root_layer, game_canvas);

    game_logic_init();
    game_draw_init();

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
    ball_pos = GPoint(Canvas_Width >> 1, Canvas_Height >> 1);
    ball_pos_x = ball_pos.x;
    ball_pos_y = ball_pos.y;
    ball_speed = ball_speed_min;
    ball_angle = rand() % 360;
    game_logic_ball_update_angle();
}

static void game_logic(void) {
    game_logic_ball();
}

static void game_logic_ball(void) {
    // TODO: Needs work
    ball_pos_x += ball_speed_cos;
    ball_pos_y += ball_speed_sin;
    ball_pos.x = ball_pos_x;
    ball_pos.y = ball_pos_y;

    ball_speed += ball_speed * ball_acceleration;
    if (ball_speed < ball_speed_min) {
        ball_speed = ball_speed_min;
    }

    if (ball_pos_x <= 0 || ball_pos_x >= Canvas_Width) {
        ball_color = GColorCyan;
        //ball_angle = 180 + 2 * 90 - ball_angle;
        ball_angle = rand() % 360;
        game_logic_ball_update_angle();
    } else if (ball_pos_y <= 0 || ball_pos_y >= Canvas_Height) {
        ball_color = GColorPurple;
        //ball_angle = 180 + 2 * 0 - ball_angle;
        ball_angle = rand() % 360;
        game_logic_ball_update_angle();
    } else {
        ball_color = GColorOrange;
    }
}

static void game_logic_ball_update_angle(void) {
    ball_speed_cos = ball_speed * (float)cos_lookup(DEG_TO_TRIGANGLE(ball_angle)) / TRIG_MAX_RATIO;
    ball_speed_sin = ball_speed * (float)sin_lookup(DEG_TO_TRIGANGLE(ball_angle)) / TRIG_MAX_RATIO;
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

    game_draw_ball(layer, ctx);
}

static void game_draw_base(Layer *layer, GContext *ctx, int player) {
    uint16_t offset_x = 0;
    uint16_t offset_y = 0;
    switch (player) {
        case Player_TopRight:
        case Player_BottomRight:
            offset_x = Canvas_Width - Base_Size;
            break;
    }
    switch (player) {
        case Player_BottomLeft:
        case Player_BottomRight:
            offset_y = Canvas_Height - Base_Size;
            break;
    }

    #if defined(PBL_COLOR)
    uint16_t quarter = Base_Size >> 2;
    switch (player) {
        case Player_TopLeft:
        case Player_TopRight:
            graphics_context_set_fill_color(ctx, GColorWindsorTan);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y, Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorOrange);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + quarter, Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorChromeYellow);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + (quarter << 1), Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorRajah);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + quarter * 3, Base_Size, quarter), 0, GCornersAll);
            break;
        case Player_BottomLeft:
        case Player_BottomRight:
            graphics_context_set_fill_color(ctx, GColorRajah);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y, Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorChromeYellow);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + quarter, Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorOrange);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + (quarter << 1), Base_Size, quarter), 0, GCornersAll);
            graphics_context_set_fill_color(ctx, GColorWindsorTan);
            graphics_fill_rect(ctx, GRect(offset_x, offset_y + quarter * 3, Base_Size, quarter), 0, GCornersAll);
            break;
    }
    #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(offset_x, offset_y, Base_Size, Base_Size), 0, GCornersAll);
    #endif

    switch (player) {
        case Player_TopRight:
        case Player_BottomRight:
            offset_x += Base_Size - Base_Inner_Size;
            break;
    }
    switch (player) {
        case Player_BottomLeft:
        case Player_BottomRight:
            offset_y += Base_Size - Base_Inner_Size;
            break;
    }

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(offset_x, offset_y, Base_Inner_Size, Base_Inner_Size), 0, GCornersAll);

    GColor color = GColorWhite;
    #if defined(PBL_COLOR)
    switch (player) {
        case Player_TopLeft:
            color = GColorPurple;
            break;
        case Player_TopRight:
            color = GColorRed;
            break;
        case Player_BottomLeft:
            color = GColorBlueMoon;
            break;
        case Player_BottomRight:
        default:
            color = GColorMintGreen;
            break;
    }
    #endif

    uint16_t padding = (Base_Inner_Size - 16) >> 1;

    // Draw characters [16x16]
    switch (player) {
        // Knight
        case Player_BottomLeft:
            game_draw_bitmap(layer, ctx, offset_x + padding, offset_y + padding, sprite_knight, GRect(0, 0, 16, 16), color);
            break;
        // Castle
        case Player_TopLeft:
        case Player_TopRight:
        case Player_BottomRight:
            game_draw_bitmap(layer, ctx, offset_x + padding, offset_y + padding, sprite_castle, GRect(0, 0, 16, 16), color);
            break;
    }
}

static void game_draw_ball(Layer *layer, GContext *ctx) {
    #if defined(PBL_COLOR)
    graphics_context_set_fill_color(ctx, ball_color);
    #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    #endif
    graphics_fill_circle(ctx, ball_pos, ball_radius);
}

static void game_draw_bitmap(Layer *layer, GContext *ctx, uint16_t pos_x, uint16_t pos_y, const uint16_t bitmap[], GRect bounds, GColor color) {
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    GRect fb_bounds = layer_get_bounds(layer);
    uint16_t x, y;

    for (y = bounds.origin.y; y < bounds.size.h; y++) {
        if (y + pos_y < fb_bounds.origin.y) continue;
        if (y + pos_y >= fb_bounds.size.h + fb_bounds.origin.y) break;
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y + pos_y);

        for (x = bounds.origin.x; x < bounds.size.w; x++) {
            if (x + pos_x < info.min_x) continue;
            if (x + pos_x >= info.max_x) break;
            if ((bitmap[y] & (1 << x)) >> x == 1) {
                #if defined(PBL_COLOR)
                memset(&info.data[x + pos_x], color.argb, 1);
                #else
                //if (color == GColorBlack) {
                //    info.data[(x + pos_x) / 8] ^= (0 ^ info.data[(x + pos_x) / 8]) & (1 << (x + pos_x)) % 8);
                //} else {
                    info.data[(x + pos_x) / 8] ^= (1 ^ info.data[(x + pos_x) / 8]) & ((1 << (x + pos_x)) % 8);
                //}
                #endif
            }
        }
    }

    graphics_release_frame_buffer(ctx, fb);
}

// Button Logic

void game_button_config(Window *window) {
    window_raw_click_subscribe(BUTTON_ID_SELECT, game_button_sel_click, game_button_sel_release, NULL);
    window_raw_click_subscribe(BUTTON_ID_UP, game_button_sel_click, game_button_sel_release, NULL);
    window_raw_click_subscribe(BUTTON_ID_DOWN, game_button_sel_click, game_button_sel_release, NULL);
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
