#include <raylib.h>
#include <raymath.h>

#include "stateflow.h"
#include "utils/button.h"
#include "utils/darray.h"
#include "utils/funcs.h"
#include "utils/input.h"
#include "utils/text.h"

static Camera2D camera;
static RenderTexture2D target;
static float scale;
static Rectangle source, dest;

static InputBox input;
static TextBox input_tb;
static bool change_screen = false;
static bool animating = false;
static bool paused = false;
static u64 frame_count;

enum {
    BUTTON_BACK_TO_EDITOR = 0,
    BUTTON_TOGGLE_ANIMATION,
    BUTTON_PAUSE,
    BUTTON_MAX
};

static Button buttons[BUTTON_MAX];

static Color bg = DARKGRAY;
static KeyboardKey navigation_keys[][4] = {
    {   KEY_A,     KEY_D,  KEY_W,    KEY_S},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN},
    {   KEY_H,     KEY_L,  KEY_K,    KEY_J},
};

static void animation_update_transforms();
static i32 animation_update_world(Vector2 mpos, GlobalState *gs, i32 handled);
static Vector2 animation_get_transformed_mouse_position(void);
static void animation_update_nodes_and_tlines(GlobalState *gs);

static void on_toggle_animation_button_clicked(GlobalState *gs);
static void on_back_to_editor_button_clicked(GlobalState *gs);
static void on_puase_button_clicked(GlobalState *gs);

static void (*on_button_clicked[BUTTON_MAX])(GlobalState *gs) = {
    on_back_to_editor_button_clicked, on_toggle_animation_button_clicked,
    on_puase_button_clicked};

static void animation_animate(GlobalState *gs);

void animation_load(GlobalState *gs) {
    change_screen = false;
    animating = false;
    paused = false;
    frame_count = 0;
    camera = (Camera2D){.target = (Vector2){0},
                        .offset = (Vector2){0},
                        .rotation = 0.0f,
                        .zoom = 1.0f};

    u64 length = darray_get_size(gs->nodes);
    for (u64 i = 0; i < length; ++i) gs->nodes[i].editing = false;
    length = darray_get_size(gs->tlines);
    for (u64 i = 0; i < length; ++i) gs->tlines[i].editing = false;

    target = LoadRenderTexture(1600, 160);

    InputBoxColors ib_colors = {.box = BLACK, .text = WHITE};

    text_box_create(&input_tb, (Rectangle){10, 10, 200, 48});
    text_box_set_color(&input_tb, WHITE);
    text_box_set_text_and_font(&input_tb, "Input", 5, gs->font);

    input_box_create(&input, (Rectangle){210, 10, 900, 48}, 256);
    input_box_set_font(&input, gs->font);
    input_box_set_colors(&input, ib_colors);

    ButtonColors button_colors = {.text = GREEN,
                                  .normal = BLUE,
                                  .down = DARKBLUE,
                                  .disabled = GRAY,
                                  .disabled_text = Fade(GREEN, 0.5),
                                  .hovered = VIOLET};

    struct {
            Rectangle rect;
            const char *name;
            u64 len;
    } button_params[BUTTON_MAX] = {
        {{1280, 10, 300, 48},  "Back to Editor", 14},
        { {450, 80, 300, 48}, "Start animation", 15},
        { {800, 80, 300, 48}, "Pause animation", 15}
    };

    for (u32 i = 0; i < BUTTON_MAX; ++i) {
        button_create(&buttons[i], button_params[i].rect);
        button_set_colors(&buttons[i], button_colors);
        button_set_text_and_font(&buttons[i], button_params[i].name,
                                 button_params[i].len, gs->font);
    }

    button_disable(&buttons[BUTTON_PAUSE]);
}

void animation_unload(GlobalState *gs) {
    UnloadRenderTexture(target);

    input_box_destroy(&input);
    text_box_destroy(&input_tb);
    for (u32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);
}

ScreenChangeType animation_update(GlobalState *gs) {
    animation_update_transforms();

    i32 handled = INPUT_NONE;

    Vector2 mpos = animation_get_transformed_mouse_position();

    handled = input_box_update(&input, mpos, handled);

    for (u32 i = 0; i < BUTTON_MAX; ++i) {
        handled = button_update(&buttons[i], mpos, handled);
        if (buttons[i].clicked) on_button_clicked[i](gs);
    }

    mpos = GetScreenToWorld2D(GetMousePosition(), camera);

    animation_update_nodes_and_tlines(gs);

    handled = animation_update_world(mpos, gs, handled);

    if (animating && !paused) {
        frame_count++;

        animation_animate(gs);
    }

    if (change_screen) return SCREEN_CHANGE;

    return SCREEN_SAME;
}

void animation_draw(GlobalState *gs) {
    ClearBackground(bg);

    BeginMode2D(camera);

    draw_grid(camera, 1.0f, 100.0f, GRAY);

    u64 length = darray_get_size(gs->tlines);
    for (u32 i = 0; i < length; ++i) tline_draw(&gs->tlines[i]);
    length = darray_get_size(gs->nodes);
    for (u32 i = 0; i < length; ++i) node_draw(&gs->nodes[i]);

    EndMode2D();

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

void animation_before_draw(GlobalState *gs) {
    BeginTextureMode(target);
    ClearBackground(GRAY);

    text_box_draw(&input_tb);
    input_box_draw(&input);
    for (u32 i = 0; i < BUTTON_MAX; ++i) button_draw(&buttons[i]);

    EndTextureMode();
}

static void animation_update_transforms() {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;

    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.3);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};
    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};
    dest.x = 0;
    dest.y = 0;
}

static i32 animation_update_world(Vector2 mpos, GlobalState *gs, i32 handled) {
    if (!IS_INPUT_HANDLED(handled, INPUT_KEYSTROKES)) {
        u32 length = (sizeof(navigation_keys) / sizeof(navigation_keys[0]));
        for (u32 i = 0; i < length; ++i) {
            if (IsKeyDown(navigation_keys[i][0])) camera.target.x -= 10;
            if (IsKeyDown(navigation_keys[i][1])) camera.target.x += 10;
            if (IsKeyDown(navigation_keys[i][2])) camera.target.y -= 10;
            if (IsKeyDown(navigation_keys[i][3])) camera.target.y += 10;
        }

        bool panning = false;
        if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)
            && !IS_INPUT_HANDLED(handled, INPUT_MIDDLE_BUTTON)) {
            panning = true;
            handled = MARK_INPUT_HANDLED(handled, INPUT_MIDDLE_BUTTON);
        } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)
                   && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
            panning = true;
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            // TraceLog(LOG_INFO, "LEFT_BUTTON is not handled!");
        }

        if (panning) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IS_INPUT_HANDLED(handled, INPUT_MOUSE_WHEEL)) return handled;

        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_WHEEL);

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            camera.offset = GetMousePosition();
            camera.target = mpos;
            float scale = 0.2f * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 48.0f);
        }

        return handled;
    }
}

static Vector2 animation_get_transformed_mouse_position(void) {
    Vector2 mpos = GetMousePosition();

    return (Vector2){.x = (mpos.x - dest.x) / scale,
                     .y = (mpos.y - dest.y) / scale};
}

static void animation_update_nodes_and_tlines(GlobalState *gs) {
    // doesn't matter, since *update_animating is called
    i32 handled = INPUT_ALL;
    Vector2 mpos = {0};
    Vector2 delta = {0};
    u64 length = darray_get_size(gs->nodes);
    for (i64 i = length - 1; i > -1; --i)
        UNUSED(node_update(&gs->nodes[i], mpos, delta, handled));

    length = darray_get_size(gs->tlines);
    for (i64 i = length - 1; i > -1; --i)
        UNUSED(tline_update(&gs->tlines[i], mpos, handled));
}

static void on_toggle_animation_button_clicked(GlobalState *gs) {
    if (animating) {
        button_set_text_and_font(&buttons[BUTTON_TOGGLE_ANIMATION],
                                 "Start animation", 15, gs->font);
        animating = false;
        button_disable(&buttons[BUTTON_PAUSE]);
    } else {
        button_set_text_and_font(&buttons[BUTTON_TOGGLE_ANIMATION],
                                 "Stop animation", 14, gs->font);
        animating = true;
        button_enable(&buttons[BUTTON_PAUSE]);
    }
}

static void on_back_to_editor_button_clicked(GlobalState *gs) {
    change_screen = true;
    gs->next_screen = &editor;
}

static void on_puase_button_clicked(GlobalState *gs) {
    if (paused) {
        button_set_text_and_font(&buttons[BUTTON_PAUSE], "Pause animation", 15,
                                 gs->font);
        paused = false;
    } else {
        button_set_text_and_font(&buttons[BUTTON_PAUSE], "Resume animation", 16,
                                 gs->font);
        paused = true;
    }
}

static void animation_animate(GlobalState *gs) {
    if (gs->fsm_type == FSM_TYPE_DFA) {
    } else if (gs->fsm_type == FSM_TYPE_NFA) {
    }
}

Screen animation = {.load = animation_load,
                    .unload = animation_unload,
                    .draw = animation_draw,
                    .before_draw = animation_before_draw,
                    .update = animation_update};
