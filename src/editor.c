#include "stateflow.h"

// raymath.h should be included after raylib.h
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <tinyfiledialogs.h>

#include "utils/button.h"
#include "utils/checkbox.h"
#include "utils/darray.h"
#include "utils/dfa.h"
#include "utils/funcs.h"
#include "utils/input.h"
#include "utils/nfa.h"
#include "utils/node.h"
#include "utils/node_selector.h"
#include "utils/text.h"
#include "utils/tline.h"

typedef enum EditroState {
    EDITOR_STATE_NODE,
    EDITOR_STATE_TRANSITION
} EditorState;

static Camera2D camera;
static RenderTexture2D target;
static float scale;
static Rectangle source, dest;
static Node *selected_node;
static TLine *selected_tline;
static EditorState editor_state;
static bool change_screen = false;
static DfaState dfa_state;
static NfaState nfa_state;
static bool show_fsm_status = false;

enum { NODE_SELECTOR_FROM = 0, NODE_SELECTOR_TO, NODE_SELECTOR_MAX };

static NodeSelector node_selectors[NODE_SELECTOR_MAX];

enum { INPUT_BOX_ALPHABET = 0, INPUT_BOX_NAME, INPUT_BOX_MAX };

static InputBox input_boxes[INPUT_BOX_MAX];

enum { CHECK_BOX_INITIAL_STATE = 0, CHECK_BOX_ACCEPTING_STATE, CHECK_BOX_MAX };

static CheckBox check_boxes[CHECK_BOX_MAX];

enum {
    TEXT_BOX_ALPHABET = 0,
    TEXT_BOX_NAME,
    TEXT_BOX_INITIAL_STATE,
    TEXT_BOX_ACCEPTING_STATE,
    TEXT_BOX_MAX
};

static TextBox text_boxes[TEXT_BOX_MAX];

enum { BUTTON_CHANGE_MODE = 0, BUTTON_SIMULATE, BUTTON_SAVE, BUTTON_MAX };

static Button buttons[BUTTON_MAX];

static void on_change_mode_button_clicked(GlobalState *gs);
static void on_simulate_button_clicked(GlobalState *gs);
static void on_save_button_clicked(GlobalState *gs);

void (*on_button_clicked[BUTTON_MAX])(GlobalState *gs) = {
    on_change_mode_button_clicked, on_simulate_button_clicked,
    on_save_button_clicked};

static Color bg;
static KeyboardKey navigation_keys[][4] = {
    {   KEY_A,     KEY_D,  KEY_W,    KEY_S},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN},
    {   KEY_H,     KEY_L,  KEY_K,    KEY_J},
};

enum { TRT_FROM, TRT_INPUTS, TRT_TO, TRT_MAX };

static TextBox tr_texts[TRT_MAX];
static InputBox tr_input;
static Button tr_button;

static i32 editor_update_world(Vector2 mpos, GlobalState *gs, i32 handled);

static Vector2 editor_get_transformed_mouse_position(void);

static void editor_update_transforms(void);

static i32 editor_update_nodes_and_tlines(GlobalState *gs, Vector2 mpos,
                                          Vector2 delta, bool update_nodes,
                                          i32 handled);

static void on_transition_add_button_clicked(GlobalState *gs);

void editor_load(GlobalState *gs) {
    bg = DARKGRAY;
    change_screen = false;
    camera = (Camera2D){
        .target = (Vector2){.x = 0, .y = 0},
        .offset = (Vector2){.x = 0, .y = 0},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    selected_node = NULL;
    selected_tline = NULL;

    u64 length = darray_get_size(gs->nodes);
    for (u64 i = 0; i < length; ++i) gs->nodes[i].editing = true;
    length = darray_get_size(gs->tlines);
    for (u64 i = 0; i < length; ++i) gs->tlines[i].editing = true;

    target = LoadRenderTexture(1600, 160);

    editor_state = EDITOR_STATE_NODE;

    struct {
            Rectangle rect;
            const char *name;
            u32 len;
    } text_params[TEXT_BOX_MAX] = {
        { {10, 10, 200, 48},        "Alphabet",  8},
        { {10, 90, 100, 48},            "Name",  4},
        {{510, 90, 350, 48},   "Initial state", 13},
        {{940, 90, 350, 48}, "Accepting state", 15},
    };

    for (i32 i = 0; i < TEXT_BOX_MAX; ++i) {
        text_box_create(&text_boxes[i], text_params[i].rect);
        text_box_set_color(&text_boxes[i], WHITE);
        text_box_set_text_and_font(&text_boxes[i], text_params[i].name,
                                   text_params[i].len, gs->font);
    }

    struct {
            Rectangle rect;
            u32 max_len;
    } input_params[INPUT_BOX_MAX] = {
        {{220, 10, 470, 48}, 100},
        {{120, 90, 370, 48},  15}
    };

    for (i32 i = 0; i < INPUT_BOX_MAX; ++i) {
        input_box_create(&input_boxes[i], input_params[i].rect,
                         input_params[i].max_len);
        input_box_set_colors(&input_boxes[i],
                             (InputBoxColors){.box = BLACK, .text = WHITE});
        input_box_set_font(&input_boxes[i], gs->font);
    }

    input_box_set_text(&input_boxes[INPUT_BOX_ALPHABET], gs->alphabet,
                       gs->alphabet_len);

    Rectangle check_box_rects[CHECK_BOX_MAX] = {
        { 870, 90, 50, 48},
        {1300, 90, 50, 48}
    };

    for (i32 i = 0; i < CHECK_BOX_MAX; ++i) {
        check_box_create(&check_boxes[i], check_box_rects[i]);
    }

    struct {
            Rectangle rect;
            const char *text;
            u32 len;
    } button_params[BUTTON_MAX] = {
        { {740, 10, 250, 48}, "Transition", 10},
        {{1020, 10, 250, 48},   "Simulate",  8},
        {{1300, 10, 250, 48},       "Save",  4}
    };

    ButtonColors button_colors = {.text = GREEN,
                                  .normal = BLUE,
                                  .down = DARKBLUE,
                                  .disabled = GRAY,
                                  .disabled_text = Fade(GREEN, 0.5),
                                  .hovered = VIOLET};

    for (i32 i = 0; i < BUTTON_MAX; ++i) {
        button_create(&buttons[i], button_params[i].rect);
        button_set_colors(&buttons[i], button_colors);
        button_set_text_and_font(&buttons[i], button_params[i].text,
                                 button_params[i].len, gs->font);
    }

    struct {
            Rectangle rect;
            const char *name;
            u32 len;
    } trt_params[TRT_MAX] = {
        { {10, 80, 150, 48}, "From: ", 6},
        {{330, 80, 150, 48}, "Input:", 6},
        {{710, 80, 150, 48},   "To: ", 4}
    };

    for (u32 i = 0; i < TRT_MAX; ++i) {
        text_box_create(&tr_texts[i], trt_params[i].rect);
        text_box_set_color(&tr_texts[i], WHITE);
        text_box_set_text_and_font(&tr_texts[i], trt_params[i].name,
                                   trt_params[i].len, gs->font);
    }

    input_box_create(&tr_input, (Rectangle){490, 90, 200, 48}, 100);
    input_box_set_colors(&tr_input,
                         (InputBoxColors){.box = BLACK, .text = WHITE});
    input_box_set_font(&tr_input, gs->font);

    struct {
            Rectangle rect;
    } selector_params[NODE_SELECTOR_MAX] = {{{160, 90, 150, 48}},
                                            {{870, 90, 150, 48}}};

    for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i) {
        node_selector_create(&node_selectors[i], selector_params[i].rect);
        node_selector_set_font(&node_selectors[i], gs->font);
    }

    button_create(&tr_button, (Rectangle){1040, 90, 150, 48});
    button_set_colors(&tr_button, button_colors);
    button_set_text_and_font(&tr_button, "Add", 3, gs->font);
}

void editor_unload(GlobalState *gs) {
    UNUSED(gs);
    for (i32 i = 0; i < TEXT_BOX_MAX; ++i) text_box_destroy(&text_boxes[i]);
    for (i32 i = 0; i < INPUT_BOX_MAX; ++i) input_box_destroy(&input_boxes[i]);
    for (i32 i = 0; i < CHECK_BOX_MAX; ++i) check_box_destroy(&check_boxes[i]);
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);
    for (u32 i = 0; i < TRT_MAX; ++i) text_box_destroy(&tr_texts[i]);
    input_box_destroy(&tr_input);
    for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
        node_selector_destroy(&node_selectors[i]);

    button_destroy(&tr_button);

    UnloadRenderTexture(target);
}

ScreenChangeType editor_update(GlobalState *gs) {
    editor_update_transforms();

    // NOTE: Screen coordinates

    bool update_nodes = true;
    i32 handled = INPUT_NONE;
    Vector2 mpos = editor_get_transformed_mouse_position();
    for (i32 i = 0; i < BUTTON_MAX; ++i) {
        handled = button_update(&buttons[i], mpos, handled);
        if (buttons[i].clicked) on_button_clicked[i](gs);
    }

    i32 menu_max = selected_node && editor_state == EDITOR_STATE_NODE
                     ? INPUT_BOX_MAX
                     : INPUT_BOX_NAME;
    for (i32 i = 0; i < menu_max; ++i)
        handled = input_box_update(&input_boxes[i], mpos, handled);

    if (editor_state == EDITOR_STATE_TRANSITION) {
        bool show_add_button = true;
        handled = input_box_update(&tr_input, mpos, handled);
        for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i) {
            handled = node_selector_update(
                &node_selectors[i], gs->nodes, mpos,
                GetScreenToWorld2D(GetMousePosition(), camera), handled);
            if (node_selectors[i].selected) update_nodes = false;
            if (node_selectors[i].node == NULL) show_add_button = false;
        }

        u32 len;
        UNUSED(input_box_get_text(&tr_input, &len));
        if (!len) show_add_button = false;
        // TraceLog(LOG_INFO, "Length = %d", len);

        if (show_add_button) button_enable(&tr_button);

        else button_disable(&tr_button);
        handled = button_update(&tr_button, mpos, handled);
        if (tr_button.clicked) on_transition_add_button_clicked(gs);
    }

    if (selected_node && editor_state == EDITOR_STATE_NODE) {
        u32 len;
        const char *name =
            input_box_get_text(&input_boxes[INPUT_BOX_NAME], &len);
        node_set_name(selected_node, name, len);

        for (i32 i = 0; i < CHECK_BOX_MAX; ++i)
            handled = check_box_update(&check_boxes[i], mpos, handled);

        selected_node->initial_state =
            check_boxes[CHECK_BOX_INITIAL_STATE].checked;
        selected_node->accepting_state =
            check_boxes[CHECK_BOX_ACCEPTING_STATE].checked;
    }

    // Works, but feel wierd
    // i32 should_handle = INPUT_LEFT_BUTTON | INPUT_RIGHT_BUTTON
    //                   | INPUT_MIDDLE_BUTTON | INPUT_MOUSE_POSITION
    //                   | INPUT_MOUSE_WHEEL;
    // if (CheckCollisionPointRec(mpos, (Rectangle){0, 0,
    // target.texture.width,
    //                                              target.texture.height}))
    //                                              {
    //     // If clicked on the menu like thing, don't pass it to editor
    //     TraceLog(LOG_INFO, "(%f, %f)", mpos.x, mpos.y);
    //     handled = MARK_INPUT_HANDLED(handled, should_handle);
    // }

    mpos = GetScreenToWorld2D(GetMousePosition(), camera);

    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, 1.0f / camera.zoom);
    // TraceLog(LOG_INFO, "Delta: (%f, %f)", delta.x, delta.y);

    handled =
        editor_update_nodes_and_tlines(gs, mpos, delta, update_nodes, handled);
    // TraceLog(LOG_INFO, "nodes = %d", handled);

    handled = editor_update_world(mpos, gs, handled);
    // TraceLog(LOG_INFO, "world = %d", handled);

    if (change_screen) return SCREEN_CHANGE;
    return SCREEN_SAME;
}

void editor_draw(GlobalState *gs) {
    ClearBackground(bg);
    BeginMode2D(camera);

    draw_grid(camera, 1.0f, 100.0f, GRAY);

    // DrawText(gs->fsm_type == FSM_TYPE_DFA ? "DFA EDITOR!" : "NFA EDITOR",
    // 5000,
    //          200, 32, WHITE);

    u64 length = darray_get_size(gs->tlines);
    for (u32 i = 0; i < length; ++i) tline_draw(&gs->tlines[i]);
    length = darray_get_size(gs->nodes);
    for (u32 i = 0; i < length; ++i) node_draw(&gs->nodes[i]);
    if (selected_tline && editor_state == EDITOR_STATE_TRANSITION)
        tline_draw(selected_tline);
    if (selected_node && editor_state == EDITOR_STATE_NODE)
        node_draw(selected_node);

    EndMode2D();

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
    // DrawFPS(10, 10);
    // if (selected_node)
    //     DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f,
    //     WHITE);

    if (!show_fsm_status) return;
    Vector2 pos = {.x = 10, .y = GetScreenHeight() - 25};
    if (gs->fsm_type == FSM_TYPE_DFA) {
        switch (dfa_state) {
            case DFA_STATE_EMPTY_ALPHABET:
                DrawTextEx(gs->font, "Alphabet is not given!", pos, 24, 1.0f,
                           RED);
                break;
            case DFA_STATE_NO_INITIAL_STATE:
                DrawTextEx(gs->font, "Initial state is not selected!", pos, 24,
                           1.0f, RED);
                break;
            case DFA_STATE_NO_ACCEPTING_STATE:
                DrawTextEx(gs->font, "There is no accepting state!", pos, 24,
                           1.0f, RED);
                break;
            case DFA_STATE_INPUT_INVALID:
                DrawTextEx(gs->font, "Input of a state is not in the alphabet",
                           pos, 24, 1.0f, RED);
                break;
            case DFA_STATE_REQUIRE_ALL_INPUT_TRANSITIONS:
                DrawTextEx(gs->font,
                           "Every state must have all possible transitions "
                           "defined!",
                           pos, 24, 1.0f, RED);
                break;
            case DFA_STATE_MULTIPLE_TRANSITIONS_DEFINED:
                DrawTextEx(gs->font,
                           "Multiple transitions having same input are not "
                           "allowed!",
                           pos, 24, 1.0f, RED);
                break;
            case DFA_STATE_ACCEPTING_STATE_NOT_REACHABLE:
                DrawTextEx(gs->font, "No path to reach accepting state!", pos,
                           24, 1.0f, RED);
                break;
            case DFA_STATE_OK:
            default:
                // DrawTextEx(gs->font, "DFA is configured correctly!", pos, 24,
                //            1.0f, GREEN);
                break;
        }
    } else if (gs->fsm_type == FSM_TYPE_NFA) {
        switch (nfa_state) {
            case NFA_STATE_EMPTY_ALPHABET:
                DrawTextEx(gs->font, "Alphabet is not given!", pos, 24, 1.0f,
                           RED);
                break;
            case NFA_STATE_NO_INITIAL_STATE:
                DrawTextEx(gs->font, "Initial state is not selected!", pos, 24,
                           1.0f, RED);
                break;
            case NFA_STATE_NO_ACCEPTING_STATE:
                DrawTextEx(gs->font, "There is no accepting state!", pos, 24,
                           1.0f, RED);
                break;
            case NFA_STATE_INPUT_INVALID:
                DrawTextEx(gs->font, "Input of a state is not in the alphabet",
                           pos, 24, 1.0f, RED);
                break;
            case NFA_STATE_ACCEPTING_STATE_NOT_REACHABLE:
                DrawTextEx(gs->font, "No path to reach accepting state!", pos,
                           24, 1.0f, RED);
                break;
            case NFA_STATE_OK:
            default:
                // DrawTextEx(gs->font, "NFA is configured correctly!", pos, 24,
                //            1.0f, GREEN);
                break;
        }
    }
}

void editor_before_draw(GlobalState *gs) {
    UNUSED(gs);
    BeginTextureMode(target);

    ClearBackground(GRAY);

    for (i32 i = 0; i < BUTTON_MAX; ++i) button_draw(&buttons[i]);
    i32 text_box_max = selected_node && editor_state == EDITOR_STATE_NODE
                         ? TEXT_BOX_MAX
                         : TEXT_BOX_NAME;

    if (editor_state == EDITOR_STATE_TRANSITION) {
        for (u32 i = 0; i < TRT_MAX; ++i) text_box_draw(&tr_texts[i]);
        input_box_draw(&tr_input);
        for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
            node_selector_draw(&node_selectors[i]);
        button_draw(&tr_button);
    }

    for (i32 i = 0; i < text_box_max; ++i) text_box_draw(&text_boxes[i]);
    i32 input_box_max = selected_node && editor_state == EDITOR_STATE_NODE
                          ? INPUT_BOX_MAX
                          : INPUT_BOX_NAME;
    for (i32 i = 0; i < input_box_max; ++i) input_box_draw(&input_boxes[i]);

    if (selected_node && editor_state == EDITOR_STATE_NODE)
        for (i32 i = 0; i < CHECK_BOX_MAX; ++i) check_box_draw(&check_boxes[i]);

    EndTextureMode();
}

static void editor_update_transforms(void) {
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

static i32 editor_update_world(Vector2 mpos, GlobalState *gs, i32 handled) {
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)
        && !IS_INPUT_HANDLED(handled, INPUT_RIGHT_BUTTON)
        && editor_state == EDITOR_STATE_NODE) {
        handled = MARK_INPUT_HANDLED(handled, MOUSE_BUTTON_RIGHT);
        Node node;
        node_create(&node, mpos);
        node_set_font(&node, gs->font, 32);
        node.editing = true;
        darray_push(&gs->nodes, node);
    }

    if (!IS_INPUT_HANDLED(handled, INPUT_KEYSTROKES)) {
        u32 length = (sizeof(navigation_keys) / sizeof(navigation_keys[0]));
        for (u32 i = 0; i < length; ++i) {
            if (IsKeyDown(navigation_keys[i][0])) camera.target.x -= 10;
            if (IsKeyDown(navigation_keys[i][1])) camera.target.x += 10;
            if (IsKeyDown(navigation_keys[i][2])) camera.target.y -= 10;
            if (IsKeyDown(navigation_keys[i][3])) camera.target.y += 10;
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            gs->next_screen = &menu;
            change_screen = true;
            return handled;
        }

        if (IsKeyPressed(KEY_DELETE)) {
            if (selected_node && editor_state == EDITOR_STATE_NODE) {
                u64 len = darray_get_size(gs->nodes);
                for (u32 i = 0; i < len; ++i) {
                    if (&gs->nodes[i] == selected_node) {
                        node_destroy(selected_node);
                        selected_node = NULL;
                        darray_pop_at(&gs->nodes, i, NULL);
                        break;
                    }
                }
            } else if (selected_tline
                       && editor_state == EDITOR_STATE_TRANSITION) {
                u64 len = darray_get_size(gs->tlines);
                for (u32 i = 0; i < len; ++i) {
                    if (&gs->tlines[i] == selected_tline) {
                        tline_destroy(selected_tline);
                        selected_tline = NULL;

                        for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
                            node_selectors[i].node = NULL;
                        input_box_set_text(&tr_input, NULL, 0);

                        darray_pop_at(&gs->tlines, i, NULL);
                        break;
                    }
                }
            }
        }

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

static i32 editor_update_nodes_and_tlines(GlobalState *gs, Vector2 mpos,
                                          Vector2 delta, bool update_nodes,
                                          i32 handled) {
    if (update_nodes) {
        Node *prev_selected = selected_node;
        if (selected_node) {
            handled = node_update(selected_node, mpos, delta, handled);
            if (!selected_node->selected) {
                selected_node = NULL;
                input_box_set_text(&input_boxes[INPUT_BOX_NAME], NULL, 0);
            }
        }
        u64 length = darray_get_size(gs->nodes);
        for (i32 i = length - 1; i > -1; --i) {
            if (&gs->nodes[i] == prev_selected) continue;
            handled = node_update(&gs->nodes[i], mpos, delta, handled);
            // Make sure only one state is initial state
            if (prev_selected && prev_selected->initial_state)
                gs->nodes[i].initial_state = false;

            if (gs->nodes[i].selected) {
                selected_node = &gs->nodes[i];
                input_box_set_text(&input_boxes[INPUT_BOX_NAME],
                                   selected_node->name,
                                   selected_node->name_length);
                check_box_set_checked(&check_boxes[CHECK_BOX_INITIAL_STATE],
                                      selected_node->initial_state);
                check_box_set_checked(&check_boxes[CHECK_BOX_ACCEPTING_STATE],
                                      selected_node->accepting_state);
            }
        }
    }

    TLine *prev_selected = selected_tline;
    if (selected_tline) {
        handled = tline_update(selected_tline, mpos, handled);
        if (!selected_tline->selected) {
            selected_tline = NULL;
            for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
                node_selectors[i].node = NULL;
            input_box_set_text(&tr_input, NULL, 0);
        }
    }

    u64 length = darray_get_size(gs->tlines);
    for (i32 i = length - 1; i > -1; --i) {
        if (&gs->tlines[i] == prev_selected) continue;
        handled = tline_update(&gs->tlines[i], mpos, handled);
        if (gs->tlines[i].selected) {
            selected_tline = &gs->tlines[i];
            u32 len;
            const char *inputs = tline_get_inputs(selected_tline, &len);
            input_box_set_text(&tr_input, inputs, len);
            node_selectors[NODE_SELECTOR_FROM].node = selected_tline->start;
            node_selectors[NODE_SELECTOR_TO].node = selected_tline->end;
        }
    }

    return handled;
}

static Vector2 editor_get_transformed_mouse_position(void) {
    Vector2 mpos = GetMousePosition();
    return (Vector2){.x = (mpos.x - dest.x) / scale,
                     .y = (mpos.y - dest.y) / scale};
}

static void on_change_mode_button_clicked(GlobalState *gs) {
    if (editor_state == EDITOR_STATE_NODE) {
        editor_state = EDITOR_STATE_TRANSITION;
        button_set_text_and_font(&buttons[BUTTON_CHANGE_MODE], "Node", 4,
                                 gs->font);
        if (selected_node) {
            selected_node->selected = false;
            selected_node = NULL;
        }
    } else if (editor_state == EDITOR_STATE_TRANSITION) {
        editor_state = EDITOR_STATE_NODE;
        button_set_text_and_font(&buttons[BUTTON_CHANGE_MODE], "Transition", 10,
                                 gs->font);
    }
}

static void on_simulate_button_clicked(GlobalState *gs) {
    u32 len;
    const char *alphabet =
        input_box_get_text(&input_boxes[INPUT_BOX_ALPHABET], &len);
    if (!len) alphabet = NULL;

    if (alphabet) {
        char *new_alphabet = realloc(gs->alphabet, (len + 1) * sizeof(char));
        if (!new_alphabet) return;
        gs->alphabet = new_alphabet;
        for (u64 i = 0; i < len; ++i) gs->alphabet[i] = alphabet[i];
        gs->alphabet[len] = 0;
        gs->alphabet_len = len;
    }

    if (gs->fsm_type == FSM_TYPE_DFA) {
        dfa_state = is_dfa_valid(gs->nodes, gs->tlines, gs->alphabet);
        if (dfa_state != DFA_STATE_OK) {
            show_fsm_status = true;
            return;
        }

    } else if (gs->fsm_type == FSM_TYPE_NFA) {
        nfa_state = is_nfa_valid(gs->nodes, gs->tlines, gs->alphabet);
        if (nfa_state != NFA_STATE_OK) {
            show_fsm_status = true;
            return;
        }
    }

    change_screen = true;
    gs->next_screen = &animation;
}

static void on_save_button_clicked(GlobalState *gs) {
    u32 len;
    const char *alphabet =
        input_box_get_text(&input_boxes[INPUT_BOX_ALPHABET], &len);
    if (!len) alphabet = NULL;

    if (alphabet) {
        char *new_alphabet = realloc(gs->alphabet, (len + 1) * sizeof(char));
        if (!new_alphabet) return;
        gs->alphabet = new_alphabet;
        for (u64 i = 0; i < len; ++i) gs->alphabet[i] = alphabet[i];
        gs->alphabet[len] = 0;
        gs->alphabet_len = len;
    }

    const char *filters[] = {"*.fsm"};
    char *file_name =
        tinyfd_saveFileDialog("Save FSM file", NULL, 1, filters, "FSM file");

    if (!file_name) return;

    if (!store_fsm_to_file(gs, file_name))
        TraceLog(LOG_ERROR, "Failed to save!");
}

static void on_transition_add_button_clicked(GlobalState *gs) {
    u32 len;
    const char *inputs = input_box_get_text(&tr_input, &len);
    u64 tlines_length = darray_get_size(gs->tlines);
    if (!selected_tline) {
        for (u64 i = 0; i < tlines_length; ++i) {
            if ((gs->tlines[i].start == node_selectors[NODE_SELECTOR_FROM].node)
                && (gs->tlines[i].end
                    == node_selectors[NODE_SELECTOR_TO].node)) {
                tline_append_inputs(&gs->tlines[i], inputs, len);
                for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
                    node_selectors[i].node = NULL;
                input_box_set_text(&tr_input, NULL, 0);
                return;
            }
        }

        TLine tline;
        tline_create(&tline);
        tline_set_start_node(&tline, node_selectors[NODE_SELECTOR_FROM].node);
        tline_set_end_node(&tline, node_selectors[NODE_SELECTOR_TO].node);
        tline_set_inputs(&tline, inputs, len);
        tline_set_font(&tline, gs->font);
        tline.editing = true;

        darray_push(&gs->tlines, tline);
    } else {
        tline_set_start_node(selected_tline,
                             node_selectors[NODE_SELECTOR_FROM].node);
        tline_set_end_node(selected_tline,
                           node_selectors[NODE_SELECTOR_TO].node);
        tline_set_inputs(selected_tline, inputs, len);
        selected_tline = NULL;
    }
    for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i) node_selectors[i].node = NULL;
    input_box_set_text(&tr_input, NULL, 0);
}

Screen editor = {.load = editor_load,
                 .unload = editor_unload,
                 .draw = editor_draw,
                 .before_draw = editor_before_draw,
                 .update = editor_update};
