#include <raymath.h>

#include "stateflow.h"
#include "utils/button.h"
#include "utils/text.h"

static TextBox test;
static Button button;
static RenderTexture2D target;
static Rectangle source, dest;
static float scale;

enum { BUTTON_DFA = 0, BUTTON_NFA, BUTTON_LOAD, BUTTON_MAX };

static Button buttons[BUTTON_MAX];

static void on_dfa_button_clicked(GlobalState *gs);
static void on_nfa_button_clicked(GlobalState *gs);
static void on_load_button_clicked(GlobalState *gs);

void (*on_button_clicked[BUTTON_MAX])(GlobalState *gs) = {
    on_dfa_button_clicked, on_nfa_button_clicked, on_load_button_clicked};

static Color bg = DARKGRAY;

static void menu_update_transforms(void);

static Vector2 menu_get_transformed_mouse_position(void);

void menu_load(GlobalState *gs) {
    target = LoadRenderTexture(800, 800);
    float x = 250;
    float width = 300;
    float height = 32;
    Rectangle rect = {.x = 250, .y = 250, .width = 300, .height = 32};

    struct {
            char *name;
            u32 len;
    } button_texts[BUTTON_MAX] = {
        {    "Create new DFA", 14},
        {    "Create new NFA", 14},
        {"Load State Machine", 18}
    };

    ButtonColors colors = {.text = GREEN,
                           .normal = BLUE,
                           .down = DARKBLUE,
                           .disabled = GRAY,
                           .disabled_text = Fade(GREEN, 0.5),
                           .hovered = VIOLET};

    for (i32 i = 0; i < BUTTON_MAX; ++i) {
        button_create(&buttons[i], rect);
        button_set_text_and_font(&buttons[i], button_texts[i].name,
                                 button_texts[i].len, GetFontDefault());
        button_set_colors(&buttons[i], colors);
        rect.y += rect.height + 100;
    }

    menu_update_transforms();
}

void menu_unload(GlobalState *gs) {
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);

    UnloadRenderTexture(target);
}

ScreenChangeType menu_update(GlobalState *gs) {
    menu_update_transforms();

    Vector2 mpos = menu_get_transformed_mouse_position();
    i32 handled = INPUT_NONE;

    for (i32 i = 0; i < BUTTON_MAX; ++i)
        handled = button_update(&buttons[i], mpos, handled);

    for (i32 i = 0; i < BUTTON_MAX; ++i) {
        if (buttons[i].clicked) {
            on_button_clicked[i](gs);
            // Just making things work temporarily
            if (i == 0) return SCREEN_CHANGE;
            break;
        }
    }

    return SCREEN_SAME;
}

void menu_before_draw(GlobalState *gs) {
    BeginTextureMode(target);

    ClearBackground(bg);
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_draw(&buttons[i]);

    EndTextureMode();
}

void menu_draw(GlobalState *gs) {
    ClearBackground(bg);

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

static void menu_update_transforms(void) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;

    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.7);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};
    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};
    dest.x = (width - dest.width) / 2;
    dest.y = (height - dest.height) / 2;
}

static Vector2 menu_get_transformed_mouse_position(void) {
    Vector2 mpos = GetMousePosition();

    return (Vector2){.x = (mpos.x - dest.x) / scale,
                     .y = (mpos.y - dest.y) / scale};
}

static void on_dfa_button_clicked(GlobalState *gs) {
    gs->next_screen = &editor;
    // gs->next_screen = &splash_screen;
}

static void on_nfa_button_clicked(GlobalState *gs) {
    button_disable(&buttons[BUTTON_DFA]);
    // gs->next_screen = &splash_screen;
}

static void on_load_button_clicked(GlobalState *gs) {
    button_enable(&buttons[BUTTON_DFA]);
    // gs->next_screen = &splash_screen;
}

Screen menu = (Screen){.load = menu_load,
                       .unload = menu_unload,
                       .draw = menu_draw,
                       .before_draw = menu_before_draw,
                       .update = menu_update};
