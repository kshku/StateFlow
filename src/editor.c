#include <raylib.h>
#include <raymath.h>

#include "stateflow.h"
#include "utils/checkbox.h"
#include "utils/darray.h"
#include "utils/input.h"
#include "utils/node.h"
#include "utils/text.h"

static Camera2D camera;
static Node *selected;
static RenderTexture2D target;
static float scale;
static Rectangle source, dest;
static Node *nodes;  // Darray

enum { INPUT_BOX_ALPHABET = 0, INPUT_BOX_NODE_NAME, INPUT_BOX_MAX };

static InputBox input_boxes[INPUT_BOX_MAX];

enum { CHECK_BOX_INITIAL_STATE = 0, CHECK_BOX_ACCEPTING_STATE, CHECK_BOX_MAX };

static CheckBox check_boxes[CHECK_BOX_MAX];

enum {
    TEXT_BOX_ALPHABET = 0,
    TEXT_BOX_NODE_NAME,
    TEXT_BOX_INITIAL_STATE,
    TEXT_BOX_ACCEPTING_STATE,
    TEXT_BOX_MAX
};

static TextBox text_boxes[TEXT_BOX_MAX];

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

static void editor_draw_grid(float thick, float spacing, Color color);

static i32 editor_update_world(Vector2 mpos, i32 handled);

static Vector2 editor_get_transformed_mouse_position(void);

static void editor_update_transforms(void);

static i32 editor_update_nodes(Vector2 mpos, Vector2 delta, i32 handled);

void editor_load(GlobalState *gs) {
    camera = (Camera2D){
        .target = (Vector2){.x = 0, .y = 0},
        .offset = (Vector2){.x = 0, .y = 0},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    selected = NULL;

    target = LoadRenderTexture(400, 50);

    nodes = darray_create(Node);

    struct {
            Rectangle rect;
            const char *name;
            u32 len;
    } text_params[TEXT_BOX_MAX] = {
        { {20, 10, 50, 10},       "Alphabet:",  9},
        { {10, 30, 30, 10},            "Name",  4},
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
        {{90, 10, 150, 10}, 100},
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
}

void editor_unload(GlobalState *gs) {
    for (i32 i = 0; i < TEXT_BOX_MAX; ++i) text_box_destroy(&text_boxes[i]);
    for (i32 i = 0; i < INPUT_BOX_MAX; ++i) input_box_destroy(&input_boxes[i]);

    darray_destroy(nodes);
    UnloadRenderTexture(target);
}

ScreenChangeType editor_update(GlobalState *gs) {
    editor_update_transforms();

    // NOTE: Screen coordinates
    Vector2 mpos = editor_get_transformed_mouse_position();

    i32 handled = INPUT_NONE;

    i32 menu_max = selected ? INPUT_BOX_MAX : INPUT_BOX_NODE_NAME;
    for (i32 i = 0; i < menu_max; ++i) {
        handled = input_box_update(&input_boxes[i], mpos, handled);
    }

    if (selected) {
        u32 len;
        const char *name =
            input_box_get_text(&input_boxes[INPUT_BOX_NODE_NAME], &len);
        node_set_name(selected, name, len);

        for (i32 i = 0; i < CHECK_BOX_MAX; ++i)
            handled = check_box_update(&check_boxes[i], mpos, handled);

        selected->initial_state = check_boxes[CHECK_BOX_INITIAL_STATE].checked;
        selected->accepting_state =
            check_boxes[CHECK_BOX_ACCEPTING_STATE].checked;
    }

    mpos = GetScreenToWorld2D(GetMousePosition(), camera);

    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, 1.0f / camera.zoom);
    // TraceLog(LOG_INFO, "Delta: (%f, %f)", delta.x, delta.y);

    handled = editor_update_nodes(mpos, delta, handled);
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

    u64 length = darray_get_size(nodes);
    for (u32 i = 0; i < length; ++i) node_draw(&nodes[i]);
    if (selected) node_draw(selected);

    EndMode2D();

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
    // DrawFPS(10, 10);
    // if (selected)
    //     DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f,
    //     WHITE);
}

void editor_before_draw(GlobalState *gs) {
    BeginTextureMode(target);

    ClearBackground(GRAY);

    i32 text_box_max = selected ? TEXT_BOX_MAX : TEXT_BOX_NODE_NAME;
    for (i32 i = 0; i < text_box_max; ++i) text_box_draw(&text_boxes[i]);
    i32 input_box_max = selected ? INPUT_BOX_MAX : INPUT_BOX_NODE_NAME;
    for (i32 i = 0; i < input_box_max; ++i) input_box_draw(&input_boxes[i]);

    if (selected)
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

static i32 editor_update_nodes(Vector2 mpos, Vector2 delta, i32 handled) {
    // Update the selected node only if exists
    // if (selected) {
    //     switch (node_update(selected, mpos, delta)) {
    //         case NODE_NOT_AFFECTED:
    //             if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    //                 // node_unlock_state(selected);
    //                 selected = NULL;
    //                 return handled;
    //             }
    //             break;
    //         case NODE_CLICKED:
    //         case NODE_MOVING:
    //         case NODE_HOVERED:
    //         default:
    //             return handled;
    //     }

    //     return false;
    // }

    Node *prev_selected = selected;
    if (selected) {
        handled = node_update(selected, mpos, delta, handled);
        if (!selected->selected) selected = NULL;
    }
    u64 length = darray_get_size(nodes);
    for (i32 i = length - 1; i > -1; --i) {
        if (&nodes[i] == prev_selected) continue;
        handled = node_update(&nodes[i], mpos, delta, handled);
        // Make sure only one state is initial state
        if (prev_selected && prev_selected->initial_state)
            nodes[i].initial_state = false;

        if (nodes[i].selected) {
            selected = &nodes[i];
            input_box_set_text(&input_boxes[INPUT_BOX_NODE_NAME],
                               selected->name, selected->name_length);
            check_box_set_checked(&check_boxes[CHECK_BOX_INITIAL_STATE],
                                  selected->initial_state);
            check_box_set_checked(&check_boxes[CHECK_BOX_ACCEPTING_STATE],
                                  selected->accepting_state);
        }
    }

    return handled;

    // bool handled = false;
    // Loop in reverse order, since while drawing last node is drawn on top
    // for (i32 i = length - 1; i >= 0; --i) {
    //     if (handled) {
    //         node_set_state(&nodes[i], NODE_STATE_NORMAL);
    //         continue;
    //     }

    //     switch (node_update(&nodes[i], mpos, delta)) {
    //         case NODE_NOT_AFFECTED:
    //             break;
    //         case NODE_CLICKED:
    //         case NODE_MOVING:
    //             selected = &nodes[i];
    //             // node_lock_state(selected, NODE_STATE_DOWN);
    //         case NODE_HOVERED:
    //         default:
    //             // handled = true;
    //     }
    // }

    // return handled;
}

static Vector2 editor_get_transformed_mouse_position(void) {
    Vector2 mpos = GetMousePosition();
    return (Vector2){.x = (mpos.x - dest.x) / scale,
                     .y = (mpos.y - dest.y) / scale};
}

Screen editor = {.load = editor_load,
                 .unload = editor_unload,
                 .draw = editor_draw,
                 .before_draw = editor_before_draw,
                 .update = editor_update};
