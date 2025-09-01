#include <raylib.h>
#include <raymath.h>

#include "stateflow.h"
#include "utils/button.h"
#include "utils/checkbox.h"
#include "utils/darray.h"
#include "utils/input.h"
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
static Node *nodes;  // Darray
static Node *selected_node;
static TLine *tlines;  // Darray
static TLine *selected_tline;
static EditorState editor_state;

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

static Color bg = DARKGRAY;
static KeyboardKey navigation_keys[][4] = {
    {   KEY_A,     KEY_D,  KEY_W,    KEY_S},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN},
    {   KEY_H,     KEY_L,  KEY_K,    KEY_J},
};
static NodeColors node_colors = {.text = GREEN,
                                 .normal = BLUE,
                                 .hovered = DARKBLUE,
                                 .down = VIOLET,
                                 .highlighted = YELLOW};

enum { TRT_FROM, TRT_INPUTS, TRT_TO, TRT_MAX };

static TextBox tr_texts[TRT_MAX];
static InputBox tr_input;

static void editor_draw_grid(float thick, float spacing, Color color);

static i32 editor_update_world(Vector2 mpos, i32 handled);

static Vector2 editor_get_transformed_mouse_position(void);

static void editor_update_transforms(void);

static i32 editor_update_nodes_and_tlines(Vector2 mpos, Vector2 delta,
                                          bool update_nodes, i32 handled);

void editor_load(GlobalState *gs) {
    camera = (Camera2D){
        .target = (Vector2){.x = 0, .y = 0},
        .offset = (Vector2){.x = 0, .y = 0},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    selected_node = NULL;
    selected_tline = NULL;

    target = LoadRenderTexture(400, 50);

    editor_state = EDITOR_STATE_NODE;
    nodes = darray_create(Node);
    tlines = darray_create(TLine);

    struct {
            Rectangle rect;
            const char *name;
            u32 len;
    } text_params[TEXT_BOX_MAX] = {
        { {10, 10, 50, 10},        "Alphabet",  8},
        { {10, 30, 40, 10},            "Name",  4},
        {{160, 30, 60, 11},   "Initial state", 13},
        {{250, 30, 80, 11}, "Accepting state", 15},
    };

    for (i32 i = 0; i < TEXT_BOX_MAX; ++i) {
        text_box_create(&text_boxes[i], text_params[i].rect);
        text_box_set_color(&text_boxes[i], WHITE);
        text_box_set_text_and_font(&text_boxes[i], text_params[i].name,
                                   text_params[i].len, GetFontDefault());
    }

    struct {
            Rectangle rect;
            u32 max_len;
    } input_params[INPUT_BOX_MAX] = {
        {{60, 10, 130, 10}, 100},
        {{50, 30, 100, 10},  15}
    };

    for (i32 i = 0; i < INPUT_BOX_MAX; ++i) {
        input_box_create(&input_boxes[i], input_params[i].rect,
                         input_params[i].max_len);
        input_box_set_colors(&input_boxes[i],
                             (InputBoxColors){.box = BLACK, .text = WHITE});
        input_box_set_font(&input_boxes[i], GetFontDefault());
    }

    Rectangle check_box_rects[CHECK_BOX_MAX] = {
        {230, 30, 10, 10},
        {340, 30, 10, 10}
    };

    for (i32 i = 0; i < CHECK_BOX_MAX; ++i) {
        check_box_create(&check_boxes[i], check_box_rects[i]);
        check_box_set_color(&check_boxes[i], GREEN);
    }

    struct {
            Rectangle rect;
            const char *text;
            u32 len;
    } button_params[BUTTON_MAX] = {
        {{200, 10, 60, 10}, "Transition", 10},
        {{265, 10, 60, 10},   "Simulate",  8},
        {{330, 10, 60, 10},       "Save",  4}
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
                                 button_params[i].len, GetFontDefault());
    }

    struct {
            Rectangle rect;
            const char *name;
            u32 len;
    } trt_params[TRT_MAX] = {
        { {10, 30, 30, 10}, "From: ", 6},
        {{100, 30, 30, 10}, "Input:", 6},
        {{260, 30, 20, 10},   "To: ", 4}
    };

    for (u32 i = 0; i < TRT_MAX; ++i) {
        text_box_create(&tr_texts[i], trt_params[i].rect);
        text_box_set_color(&tr_texts[i], WHITE);
        text_box_set_text_and_font(&tr_texts[i], trt_params[i].name,
                                   trt_params[i].len, GetFontDefault());
    }

    input_box_create(&tr_input, (Rectangle){140, 30, 100, 10}, 100);
    input_box_set_colors(&tr_input,
                         (InputBoxColors){.box = BLACK, .text = WHITE});
    input_box_set_font(&tr_input, GetFontDefault());

    struct {
            Rectangle rect;
    } selector_params[NODE_SELECTOR_MAX] = {
        { 45, 30, 50, 10},
        {285, 30, 50, 10}
    };

    for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i) {
        node_selector_create(&node_selectors[i], selector_params[i].rect);
        node_selector_set_font(&node_selectors[i], GetFontDefault());
    }
}

void editor_unload(GlobalState *gs) {
    for (i32 i = 0; i < TEXT_BOX_MAX; ++i) text_box_destroy(&text_boxes[i]);
    for (i32 i = 0; i < INPUT_BOX_MAX; ++i) input_box_destroy(&input_boxes[i]);
    for (i32 i = 0; i < CHECK_BOX_MAX; ++i) check_box_destroy(&check_boxes[i]);
    for (i32 i = 0; i < BUTTON_MAX; ++i) button_destroy(&buttons[i]);
    for (u32 i = 0; i < TRT_MAX; ++i) text_box_destroy(&tr_texts[i]);
    input_box_destroy(&tr_input);
    for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i)
        node_selector_destroy(&node_selectors[i]);

    darray_destroy(nodes);
    darray_destroy(tlines);

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
        handled = input_box_update(&tr_input, mpos, handled);
        for (u32 i = 0; i < NODE_SELECTOR_MAX; ++i) {
            handled = node_selector_update(
                &node_selectors[i], nodes, mpos,
                GetScreenToWorld2D(GetMousePosition(), camera), handled);
            if (node_selectors[i].selected) update_nodes = false;
        }
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
        editor_update_nodes_and_tlines(mpos, delta, update_nodes, handled);
    // TraceLog(LOG_INFO, "nodes = %d", handled);

    handled = editor_update_world(mpos, handled);
    // TraceLog(LOG_INFO, "world = %d", handled);

    return SCREEN_SAME;
}

void editor_draw(GlobalState *gs) {
    ClearBackground(bg);
    BeginMode2D(camera);

    editor_draw_grid(1.0f, 100.0f, GRAY);

    DrawText("DFA EDITOR!", 5000, 200, 32, WHITE);

    u64 length = darray_get_size(tlines);
    for (u32 i = 0; i < length; ++i) tline_draw(&tlines[i]);
    length = darray_get_size(nodes);
    for (u32 i = 0; i < length; ++i) node_draw(&nodes[i]);
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
}

void editor_before_draw(GlobalState *gs) {
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

    scale = CLAMP_MIN(scale, 1.0);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};
    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};
    dest.x = 0;
    dest.y = 0;
}

static void editor_draw_grid(float thick, float spacing, Color color) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();

    thick /= camera.zoom;

    // Get the world position
    Vector2 top_left = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottom_right = GetScreenToWorld2D((Vector2){width, height}, camera);

    // align
    float start_x = floorf(top_left.x / spacing) * spacing;
    float end_x = floorf(bottom_right.x / spacing) * spacing;
    float start_y = floorf(top_left.y / spacing) * spacing;
    float end_y = floorf(bottom_right.y / spacing) * spacing;

    for (float x = start_x; x <= end_x; x += spacing)
        DrawLineEx((Vector2){x, top_left.y}, (Vector2){x, bottom_right.y},
                   thick, color);

    for (float y = start_y; y <= end_y; y += spacing)
        DrawLineEx((Vector2){top_left.x, y}, (Vector2){bottom_right.x, y},
                   thick, color);
}

static i32 editor_update_world(Vector2 mpos, i32 handled) {
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)
        && !IS_INPUT_HANDLED(handled, INPUT_RIGHT_BUTTON)) {
        handled = MARK_INPUT_HANDLED(handled, MOUSE_BUTTON_RIGHT);
        Node node;
        node_create(&node, mpos);
        node_set_colors(&node, node_colors);
        node_set_font(&node, GetFontDefault(), 32);
        darray_push(&nodes, node);
    }

    if (!IS_INPUT_HANDLED(handled, INPUT_KEYSTROKES)) {
        u32 length = (sizeof(navigation_keys) / sizeof(navigation_keys[0]));
        for (u32 i = 0; i < length; ++i) {
            if (IsKeyDown(navigation_keys[i][0])) camera.target.x -= 10;
            if (IsKeyDown(navigation_keys[i][1])) camera.target.x += 10;
            if (IsKeyDown(navigation_keys[i][2])) camera.target.y -= 10;
            if (IsKeyDown(navigation_keys[i][3])) camera.target.y += 10;
        }

        if (IsKeyPressed(KEY_DELETE)) {
            if (selected_node && editor_state == EDITOR_STATE_NODE) {
                u64 len = darray_get_size(nodes);
                for (u32 i = 0; i < len; ++i) {
                    if (&nodes[i] == selected_node) {
                        darray_pop_at(&nodes, i, NULL);
                        node_destroy(selected_node);
                        selected_node = NULL;
                        break;
                    }
                }
            } else if (selected_tline
                       && editor_state == EDITOR_STATE_TRANSITION) {
                u64 len = darray_get_size(tlines);
                for (u32 i = 0; i < len; ++i) {
                    if (&tlines[i] == selected_tline) {
                        darray_pop_at(&tlines, i, NULL);
                        tline_destroy(selected_tline);
                        selected_tline = NULL;
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
        camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
    }

    return handled;
}

static i32 editor_update_nodes_and_tlines(Vector2 mpos, Vector2 delta,
                                          bool update_nodes, i32 handled) {
    if (update_nodes) {
        Node *prev_selected = selected_node;
        if (selected_node) {
            handled = node_update(selected_node, mpos, delta, handled);
            if (!selected_node->selected) {
                selected_node = NULL;
                input_box_set_text(&input_boxes[INPUT_BOX_NAME], "", 0);
            }
        }
        u64 length = darray_get_size(nodes);
        for (i32 i = length - 1; i > -1; --i) {
            if (&nodes[i] == prev_selected) continue;
            handled = node_update(&nodes[i], mpos, delta, handled);
            // Make sure only one state is initial state
            if (prev_selected && prev_selected->initial_state)
                nodes[i].initial_state = false;

            if (nodes[i].selected) {
                selected_node = &nodes[i];
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

    u64 length = darray_get_size(tlines);
    for (i32 i = length - 1; i > -1; --i) {
        handled = tline_update(&tlines[i], mpos, handled);
        if (tlines[i].selected) selected_tline = &tlines[i];
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
                                 GetFontDefault());
    } else if (editor_state == EDITOR_STATE_TRANSITION) {
        editor_state = EDITOR_STATE_NODE;
        button_set_text_and_font(&buttons[BUTTON_CHANGE_MODE], "Transition", 10,
                                 GetFontDefault());
    }
}

static void on_simulate_button_clicked(GlobalState *gs) {
    UNUSED(gs);
}

static void on_save_button_clicked(GlobalState *gs) {
    UNUSED(gs);
}

Screen editor = {.load = editor_load,
                 .unload = editor_unload,
                 .draw = editor_draw,
                 .before_draw = editor_before_draw,
                 .update = editor_update};
