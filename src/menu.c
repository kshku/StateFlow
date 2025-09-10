#include <raymath.h>
#include <stdlib.h>
#include <tinyfiledialogs.h>

#include "stateflow.h"
#include "utils/button.h"
#include "utils/darray.h"
#include "utils/funcs.h"
#include "utils/node.h"
#include "utils/text.h"
#include "utils/tline.h"

static RenderTexture2D target;
static Rectangle source, dest;
static float scale;
static bool change_screen = false;

enum { BUTTON_DFA = 0, BUTTON_NFA, BUTTON_LOAD, BUTTON_MAX };

static Button buttons[BUTTON_MAX];

static void on_dfa_button_clicked(GlobalState *gs);
static void on_nfa_button_clicked(GlobalState *gs);
static void on_load_button_clicked(GlobalState *gs);

static void (*on_button_clicked[BUTTON_MAX])(GlobalState *gs) = {
    on_dfa_button_clicked, on_nfa_button_clicked, on_load_button_clicked};

static Color bg;

static void menu_update_transforms(void);

static Vector2 menu_get_transformed_mouse_position(void);

void menu_load(GlobalState *gs) {
    bg = DARKGRAY;
    change_screen = false;
    target = LoadRenderTexture(1600, 1600);
    Rectangle rect;
    rect.width = (target.texture.width / 3) * 2;
    rect.height = target.texture.height / 15;
    rect.x = (target.texture.width - rect.width) / 2;
    rect.y = (float)target.texture.height / 4;
    float diff = (float)target.texture.height / 6;

    struct {
            char *name;
            u32 len;
    } button_texts[BUTTON_MAX] = {
        {"Create new DFA", 14},
        {"Create new NFA", 14},
        {      "Load FSM",  8}
    };

    for (i32 i = 0; i < BUTTON_MAX; ++i) {
        button_create(&buttons[i], rect);
        button_set_text_and_font(&buttons[i], button_texts[i].name,
                                 button_texts[i].len, gs->font);
        rect.y += rect.height + diff;
    }

    menu_update_transforms();

    u64 length = darray_get_size(gs->nodes);
    for (u64 i = 0; i < length; ++i) node_destroy(&gs->nodes[i]);
    darray_clear(gs->nodes);
    length = darray_get_size(gs->tlines);
    for (u64 i = 0; i < length; ++i) tline_destroy(&gs->tlines[i]);
    darray_clear(gs->tlines);

    free(gs->alphabet);
    gs->alphabet = NULL;
    gs->alphabet_len = 0;
}

void menu_unload(GlobalState *gs) {
    UNUSED(gs);
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);

    UnloadRenderTexture(target);
}

ScreenChangeType menu_update(GlobalState *gs) {
    menu_update_transforms();

    Vector2 mpos = menu_get_transformed_mouse_position();
    i32 handled = INPUT_NONE;

    for (i32 i = 0; i < BUTTON_MAX; ++i)
        handled = button_update(&buttons[i], mpos, handled);

    for (i32 i = 0; i < BUTTON_MAX; ++i)
        if (buttons[i].clicked) on_button_clicked[i](gs);

    if (change_screen) return SCREEN_CHANGE;
    return SCREEN_SAME;
}

void menu_before_draw(GlobalState *gs) {
    UNUSED(gs);
    BeginTextureMode(target);

    ClearBackground(bg);
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_draw(&buttons[i]);

    EndTextureMode();
}

void menu_draw(GlobalState *gs) {
    UNUSED(gs);
    ClearBackground(bg);

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

static void menu_update_transforms(void) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;

    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.3);

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
    gs->fsm_type = FSM_TYPE_DFA;
    change_screen = true;
}

static void on_nfa_button_clicked(GlobalState *gs) {
    // button_disable(&buttons[BUTTON_DFA]);
    gs->next_screen = &editor;
    gs->fsm_type = FSM_TYPE_NFA;
    change_screen = true;
}

static void on_load_button_clicked(GlobalState *gs) {
    const char *filters[] = {"*.fsm"};
    char *file_name = tinyfd_openFileDialog("Open the FSM file", NULL, 1,
                                            filters, "FSM files", 0);

    if (!file_name) return;

    if (load_fsm_from_file(gs, file_name)) {
        gs->next_screen = &editor;
        change_screen = true;
    }
}

Screen menu = {.load = menu_load,
               .unload = menu_unload,
               .draw = menu_draw,
               .before_draw = menu_before_draw,
               .update = menu_update};
