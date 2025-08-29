#include <raymath.h>

#include "stateflow.h"
#include "utils/button.h"
#include "utils/text.h"

static TextBox test;
static Button button;
static RenderTexture2D target;
static Button dfa, nfa;
static Button load;
static Rectangle source, dest;
static float scale;

static Color bg = DARKGRAY;

static void menu_update_transforms(void);

static void menu_update_mouse_position(Vector2 *mpos);

void menu_load(GlobalState *gs) {
    target = LoadRenderTexture(800, 800);
    float x = 250;
    float width = 300;
    float height = 32;
    Rectangle rect = {.x = 250, .y = 250, .width = 300, .height = 32};

    button_create(&dfa, rect);

    rect.y += rect.height + 100;
    button_create(&nfa, rect);

    rect.y += rect.height + 100;
    button_create(&load, rect);

    button_set_text_and_font(&dfa, "Create new DFA", 14, GetFontDefault());
    button_set_text_and_font(&nfa, "Create new NFA", 14, GetFontDefault());
    button_set_text_and_font(&load, "Load State Machine", 18, GetFontDefault());

    ButtonColors colors = {.text = GREEN,
                           .normal = BLUE,
                           .down = DARKBLUE,
                           .disabled = GRAY,
                           .disabled_text = Fade(GREEN, 0.5),
                           .hovered = VIOLET};

    // button_set_color(&dfa, BLUE, GREEN);
    // button_set_color(&nfa, BLUE, GREEN);
    // button_set_color(&load, BLUE, GREEN);
    button_set_colors(&dfa, colors);
    button_set_colors(&nfa, colors);
    button_set_colors(&load, colors);

    menu_update_transforms();
}

void menu_unload(GlobalState *gs) {
    button_destroy(&dfa);
    button_destroy(&nfa);
    button_destroy(&load);

    UnloadRenderTexture(target);
}

ScreenChangeType menu_update(GlobalState *gs) {
    menu_update_transforms();

    Vector2 mpos = GetMousePosition();
    menu_update_mouse_position(&mpos);

    if (button_update(&dfa, mpos)) {
        gs->next_screen = &editor;
        // gs->next_screen = &splash_screen;
        return SCREEN_CHANGE;
    }

    if (button_update(&nfa, mpos)) {
        button_disable(&dfa);
        // gs->next_screen = &splash_screen;
        // return SCREEN_CHANGE;
    }

    if (button_update(&load, mpos)) {
        button_enable(&dfa);
        // gs->next_screen = &splash_screen;
        // return SCREEN_CHANGE;
    }

    return SCREEN_SAME;
}

void menu_before_draw(GlobalState *gs) {
    BeginTextureMode(target);

    ClearBackground(bg);

    button_draw(&dfa);
    button_draw(&nfa);
    button_draw(&load);

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

static void menu_update_mouse_position(Vector2 *mpos) {
    mpos->x -= dest.x;
    mpos->y -= dest.y;
    mpos->x /= scale;
    mpos->y /= scale;
}

Screen menu = (Screen){.load = menu_load,
                       .unload = menu_unload,
                       .draw = menu_draw,
                       .before_draw = menu_before_draw,
                       .update = menu_update};
