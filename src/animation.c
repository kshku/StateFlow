#include "stateflow.h"

// raymath.h should be included after raylib.h
#include <raymath.h>

#include "utils/button.h"
#include "utils/darray.h"
#include "utils/dfa.h"
#include "utils/funcs.h"
#include "utils/input.h"
#include "utils/nfa.h"
#include "utils/strops.h"
#include "utils/text.h"

typedef enum AnimatingState {
    ANIMATING_STATE_NONE = 0,
    ANIMATING_STATE_NODE = 1 << 0,
    ANIMATING_STATE_INPUT = 1 << 1,
    ANIMATING_STATE_TLINE = 1 << 2,
    ANIMATING_STATE_RESULT = 1 << 3,
    ANIMATING_STATE_WATING = 1 << 4,
    ANIMATING_STATE_DONE = 1 << 5,
} AnimatingState;

static Camera2D camera;
static RenderTexture2D target;
static float scale;
static Rectangle source, dest;
static Node *initial_state = NULL;
static Node **current_states = NULL;
static bool invalid_input = false;
static AnimatingState anim_state, anim_prev_state, anim_next_state;

static InputBox input;
static bool change_screen = false;
static bool animating = false;
static bool paused = false;
static u64 frame_count;
// TODO: UI for changing the speed
static float speed = 1.0f;

enum Result {
    RESULT_NONE = 0,
    RESULT_ACCEPTED,
    RESULT_REJECTED,
} result = RESULT_NONE;

static Vector2 previous_input_vec, current_input_vec, next_input_vec;

static const char *input_text = NULL;
static u32 input_text_length = 0;
static i32 input_text_index = -1;

enum { TEXT_BOX_INPUT, TEXT_BOX_MAX };

TextBox text_boxes[TEXT_BOX_MAX];

enum {
    BUTTON_BACK_TO_EDITOR = 0,
    BUTTON_TOGGLE_ANIMATION,
    BUTTON_PAUSE,
    BUTTON_MAX
};

static Button buttons[BUTTON_MAX];

static Color bg;
static KeyboardKey navigation_keys[][4] = {
    {   KEY_A,     KEY_D,  KEY_W,    KEY_S},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN},
    {   KEY_H,     KEY_L,  KEY_K,    KEY_J},
};

static void animation_update_transforms(void);
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
    bg = DARKGRAY;
    change_screen = false;
    initial_state = NULL;
    anim_state = anim_next_state = anim_prev_state = ANIMATING_STATE_NONE;
    animating = false;
    paused = false;
    frame_count = 0;
    input_text_index = -1;
    invalid_input = false;
    result = RESULT_NONE;
    camera = (Camera2D){.target = (Vector2){0},
                        .offset = (Vector2){0},
                        .rotation = 0.0f,
                        .zoom = 1.0f};

    u64 length = darray_get_size(gs->nodes);
    for (u64 i = 0; i < length; ++i) {
        gs->nodes[i].editing = false;
        if (gs->nodes[i].initial_state) initial_state = &gs->nodes[i];
    }
    length = darray_get_size(gs->tlines);
    for (u64 i = 0; i < length; ++i) gs->tlines[i].editing = false;

    target = LoadRenderTexture(1600, 160);

    current_states = darray_create(Node *);

    struct {
            Rectangle rect;
            const char *text;
            u32 len;
            Color color;
    } text_box_params[TEXT_BOX_MAX] = {
        {{10, 10, 200, 48}, "Input", 5, WHITE},
    };

    for (u32 i = 0; i < TEXT_BOX_MAX; ++i) {
        text_box_create(&text_boxes[i], text_box_params[i].rect);
        text_box_set_color(&text_boxes[i], text_box_params[i].color);
        text_box_set_text_and_font(&text_boxes[i], text_box_params[i].text,
                                   text_box_params[i].len, gs->font);
    }

    input_box_create(&input, (Rectangle){210, 10, 900, 48}, 256);
    input_box_set_font(&input, gs->font);

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
        button_set_text_and_font(&buttons[i], button_params[i].name,
                                 button_params[i].len, gs->font);
    }

    button_disable(&buttons[BUTTON_PAUSE]);
}

void animation_unload(GlobalState *gs) {
    UNUSED(gs);
    UnloadRenderTexture(target);

    input_box_destroy(&input);
    for (u32 i = 0; i < TEXT_BOX_MAX; ++i) text_box_destroy(&text_boxes[i]);
    for (u32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);

    darray_destroy(current_states);
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

    if (animating) {
        if (input_text_index >= 0) {
            DrawTextEx(gs->font, TextSubtext(input_text, 0, input_text_index),
                       previous_input_vec, 48, 1.0f, WHITE);

            DrawTextEx(gs->font, TextSubtext(input_text, input_text_index, 1),
                       current_input_vec, 72, 1.0f, WHITE);
        }

        DrawTextEx(gs->font,
                   TextSubtext(input_text, input_text_index + 1,
                               input_text_length - input_text_index - 1),
                   next_input_vec, 48, 1.0f, WHITE);
    }
    if (invalid_input) {
        DrawTextEx(gs->font, "Invalid input!",
                   (Vector2){10, GetScreenHeight() - 25}, 24, 1.0f, RED);
    }

    switch (result) {
        case RESULT_ACCEPTED:
            DrawTextEx(gs->font, "Accepted",
                       (Vector2){0, GetScreenHeight() - 48}, 48, 1.0f, GREEN);
            break;
        case RESULT_REJECTED:
            DrawTextEx(gs->font, "Rejected",
                       (Vector2){0, GetScreenHeight() - 48}, 48, 1.0f, RED);
            break;
        case RESULT_NONE:
        default:
            break;
    }
}

void animation_before_draw(GlobalState *gs) {
    UNUSED(gs);
    BeginTextureMode(target);
    ClearBackground(GRAY);

    for (u32 i = 0; i < TEXT_BOX_MAX; ++i) text_box_draw(&text_boxes[i]);
    input_box_draw(&input);
    for (u32 i = 0; i < BUTTON_MAX; ++i) button_draw(&buttons[i]);

    EndTextureMode();
}

static void animation_update_transforms(void) {
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

            if (IsKeyDown(KEY_BACKSPACE)) on_back_to_editor_button_clicked(gs);

            handled = MARK_INPUT_HANDLED(handled, INPUT_KEYSTROKES);
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

    return handled;
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
    invalid_input = false;
    if (animating) {
        button_set_text_and_font(&buttons[BUTTON_TOGGLE_ANIMATION],
                                 "Start animation", 15, gs->font);
        animating = false;
        button_disable(&buttons[BUTTON_PAUSE]);
        result = RESULT_NONE;
        anim_state = anim_prev_state = anim_next_state = ANIMATING_STATE_NONE;
        input_box_enable(&input);
    } else {
        input_text_index = -1;
        result = RESULT_NONE;
        anim_state = anim_prev_state = anim_next_state = ANIMATING_STATE_NONE;

        input_text = input_box_get_text(&input, &input_text_length);
        if (!all_chars_present(gs->alphabet, input_text)) {
            invalid_input = true;
            return;
        }

        button_set_text_and_font(&buttons[BUTTON_TOGGLE_ANIMATION],
                                 "Stop animation", 14, gs->font);
        animating = true;
        button_enable(&buttons[BUTTON_PAUSE]);
        input_box_disable(&input);
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
    u64 current_states_length = darray_get_size(current_states);
    u64 tlines_length = darray_get_size(gs->tlines);

    switch (anim_state) {
        case ANIMATING_STATE_NONE:
            darray_clear(current_states);
            darray_push(&current_states, initial_state);
            current_states_length = darray_get_size(current_states);
            anim_prev_state = ANIMATING_STATE_NONE;
            anim_state = ANIMATING_STATE_WATING;
            anim_next_state = ANIMATING_STATE_INPUT;
            break;
        case ANIMATING_STATE_NODE:
            if (gs->fsm_type == FSM_TYPE_DFA) {
                Node *next_state =
                    dfa_transition(current_states[0], gs->tlines, tlines_length,
                                   input_text[input_text_index]);
                darray_pop(&current_states, NULL);
                darray_push(&current_states, next_state);
                current_states_length = darray_get_size(current_states);
            } else if (gs->fsm_type == FSM_TYPE_NFA) {
                Node **next_states = darray_create(Node *);
                for (u64 i = 0; i < current_states_length; ++i)
                    next_states = nfa_transition(current_states[i], next_states,
                                                 gs->tlines, tlines_length,
                                                 input_text[input_text_index]);
                darray_clear(current_states);
                u64 length = darray_get_size(next_states);
                for (u64 i = 0; i < length; ++i)
                    darray_push(&current_states, next_states[i]);
                current_states_length = darray_get_size(current_states);
                darray_destroy(next_states);
            }
            anim_prev_state = ANIMATING_STATE_NODE;
            anim_state = ANIMATING_STATE_WATING;
            anim_next_state = ANIMATING_STATE_INPUT;
            break;
        case ANIMATING_STATE_INPUT:
            input_text_index =
                CLAMP_MAX(input_text_index + 1, (i32)input_text_length);
            if (input_text_index == (i32)input_text_length) {
                anim_state = ANIMATING_STATE_RESULT;
                break;
            }
            anim_prev_state = ANIMATING_STATE_INPUT;
            anim_state = ANIMATING_STATE_WATING;
            anim_next_state = ANIMATING_STATE_TLINE;
            break;
        case ANIMATING_STATE_TLINE:
            anim_prev_state = ANIMATING_STATE_TLINE;
            anim_state = ANIMATING_STATE_WATING;
            anim_next_state = ANIMATING_STATE_NODE;
            break;
        case ANIMATING_STATE_RESULT:
            result = RESULT_REJECTED;
            anim_state = ANIMATING_STATE_DONE;
            for (u64 i = 0; i < current_states_length; ++i) {
                if (current_states[i]->accepting_state) {
                    result = RESULT_ACCEPTED;
                    anim_state = ANIMATING_STATE_DONE;
                    break;
                }
            }
            break;
        case ANIMATING_STATE_WATING:
            if (frame_count % (u64)(60 / speed) == 0)
                anim_state = anim_next_state;
            break;
        case ANIMATING_STATE_DONE:
        default:
            break;
    }

    for (u64 i = 0; i < current_states_length; ++i)
        current_states[i]->state = NODE_STATE_HIGHLIGHTED;

    if (anim_prev_state == ANIMATING_STATE_TLINE) {
        for (u64 i = 0; i < tlines_length; ++i) {
            for (u64 j = 0; j < current_states_length; ++j) {
                if (gs->tlines[i].start == current_states[j]) {
                    char input_str[] = {input_text[input_text_index], 0};
                    if (all_chars_present(gs->tlines[i].inputs, input_str)) {
                        gs->tlines[i].state = TLINE_STATE_HIGHLIGHTED;
                        break;
                    }
                }
            }
        }
    }

    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();

    if (input_text_index >= 0) {
        Vector2 prev_size = MeasureTextEx(
            gs->font, TextSubtext(input_text, 0, input_text_index), 48, 1.0f);
        Vector2 cur_size = MeasureTextEx(
            gs->font, TextSubtext(input_text, input_text_index, 1), 72, 1.0f);
        Vector2 next_size = MeasureTextEx(
            gs->font,
            TextSubtext(input_text, input_text_index + 1,
                        (input_text_length - input_text_index - 1)),
            48, 1.0f);

        previous_input_vec =
            (Vector2){.x = (((width - cur_size.x) / 2.0f) - prev_size.x),
                      .y = height - prev_size.y};
        current_input_vec = (Vector2){.x = previous_input_vec.x + prev_size.x,
                                      .y = height - cur_size.y};
        next_input_vec = (Vector2){.x = current_input_vec.x + cur_size.x,
                                   .y = height - next_size.y};
    } else {
        Vector2 next_size = MeasureTextEx(
            gs->font,
            TextSubtext(input_text, input_text_index + 1,
                        (input_text_length - input_text_index - 1)),
            48, 1.0f);
        next_input_vec = (Vector2){.x = (width - next_size.x) / 2.0f,
                                   .y = height - next_size.y};
    }
}

Screen animation = {.load = animation_load,
                    .unload = animation_unload,
                    .draw = animation_draw,
                    .before_draw = animation_before_draw,
                    .update = animation_update};
