#include <raylib.h>
#include <raymath.h>

#include "stateflow.h"
#include "utils/darray.h"
#include "utils/node.h"

static Camera2D camera;
static Node *selected;
static RenderTexture2D target;
static float scale;
static Rectangle source, dest;

static Color bg = DARKGRAY;
static KeyboardKey navigation_keys[][4] = {
    {   KEY_A,     KEY_D,  KEY_W,    KEY_S},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN},
    {   KEY_H,     KEY_L,  KEY_K,    KEY_J},
};
static NodeColors node_colors = {.font = GREEN,
                                 .normal = BLUE,
                                 .hovered = DARKBLUE,
                                 .selected = VIOLET,
                                 .highlighted = YELLOW};

static Node *nodes;

static void editor_draw_grid(float thick, float spacing, Color color);

static void editor_update_world(void);

static void editor_draw_editor(Node *node);

static void editor_update_transforms(void);

void editor_load(GlobalState *gs) {
    camera = (Camera2D){
        .target = (Vector2){.x = 0, .y = 0},
        .offset = (Vector2){.x = 0, .y = 0},
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    selected = NULL;

    target = LoadRenderTexture(400, 800);

    nodes = darray_create(Node);
}

void editor_unload(GlobalState *gs) {
    darray_destroy(nodes);
    UnloadRenderTexture(target);
}

ScreenChangeType editor_update(GlobalState *gs) {
    editor_update_world();
    editor_update_transforms();

    Vector2 mpos = GetMousePosition();
    mpos = GetScreenToWorld2D(mpos, camera);

    bool clicked = false;

    u64 length = darray_get_size(nodes);
    selected = NULL;
    for (u32 i = 0; i < length; ++i) {
        if (nodes[i].state == NODE_STATE_SELECTED) {
            selected = &nodes[i];
            break;
        }
        if (node_update(&nodes[i], mpos)) {
            node_lock_selected(&nodes[i]);
            selected = &nodes[i];
            clicked = true;
            break;
        }
    }

    if (!clicked && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Node node;
        node_create(&node, mpos);
        node_set_colors(&node, node_colors);
        node_set_font(&node, GetFontDefault(), 32);
        darray_push(&nodes, node);
    }

    return SCREEN_SAME;
}

void editor_draw(GlobalState *gs) {
    ClearBackground(bg);
    BeginMode2D(camera);

    editor_draw_grid(1.0f, 100.0f, GRAY);

    DrawText("DFA EDITOR!", 5000, 200, 32, WHITE);

    u64 length = darray_get_size(nodes);
    for (u32 i = 0; i < length; ++i) node_draw(&nodes[i]);

    EndMode2D();

    if (selected)
        DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

void editor_before_draw(GlobalState *gs) {
    if (!selected) return;

    BeginTextureMode(target);

    ClearBackground(GRAY);

    editor_draw_editor(selected);

    EndTextureMode();
}

static void editor_update_transforms(void) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;

    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.5);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};
    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};
    dest.x = 0;
    dest.y = (height - dest.height) / 2;
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

static void editor_update_world(void) {
    u32 length = (sizeof(navigation_keys) / sizeof(navigation_keys[0]));
    for (u32 i = 0; i < length; ++i) {
        if (IsKeyDown(navigation_keys[i][0])) camera.target.x -= 10;
        if (IsKeyDown(navigation_keys[i][1])) camera.target.x += 10;
        if (IsKeyDown(navigation_keys[i][2])) camera.target.y += 10;
        if (IsKeyDown(navigation_keys[i][3])) camera.target.y -= 10;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mouse_world_pos =
            GetScreenToWorld2D(GetMousePosition(), camera);
        camera.offset = GetMousePosition();
        camera.target = mouse_world_pos;
        float scale = 0.2f * wheel;
        camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
    }
}

static void editor_draw_editor(Node *node) {
    DrawText("Testing the thing!", 50, 50, 20, WHITE);
}

Screen editor = {.load = editor_load,
                 .unload = editor_unload,
                 .draw = editor_draw,
                 .before_draw = editor_before_draw,
                 .update = editor_update};
