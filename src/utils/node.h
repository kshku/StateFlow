#pragma once

#include <raylib.h>

#include "defines.h"

typedef enum NodeState {
    NODE_STATE_NORMAL,
    NODE_STATE_DOWN,
    NODE_STATE_HOVERED,
    NODE_STATE_HIGHLIGHTED,
} NodeState;

typedef struct NodeColors {
        Color normal;
        Color text;
        Color down;
        Color hovered;
        Color highlighted;
} NodeColors;

typedef struct Node {
        char *name;
        Font font;
        float font_size;
        Vector2 center;
        Vector2 position;
        float radius;
        NodeState state;
        NodeColors colors;
        bool editing;
        bool moving;
        bool locked;
        bool selected;
        bool pressed;
} Node;

typedef enum NodeStatus {
    NODE_NOT_AFFECTED,
    NODE_HOVERED,
    NODE_MOVING,
    NODE_CLICKED,
} NodeStatus;

// typedef enum NodeState {
//     NODE_STATE_NORMAL,
//     NODE_STATE_SELECTED,
//     NODE_STATE_HIGHLIGHTED,
//     NODE_STATE_HOVERED
// } NodeState;

// typedef struct NodeColors {
//         Color normal;
//         Color selected;
//         Color hovered;
//         Color font;
//         Color highlighted;
// } NodeColors;

// typedef struct Node {
//         char *name;
//         Font font;
//         float font_size;
//         Vector2 center;
//         Vector2 position;
//         NodeColors colors;
//         float radius;
//         NodeState state;
//         bool locked;
//         bool moving;
// } Node;

void node_create(Node *n, Vector2 center);

void node_destroy(Node *n);

void node_set_name(Node *n, char *name, u32 len);

void node_set_font(Node *n, Font font, float font_size);

void node_set_colors(Node *n, NodeColors colors);

void node_draw(Node *n);

NodeStatus node_update(Node *n, Vector2 mpos, Vector2 delta);

void node_lock_state(Node *n, NodeState state);
void node_unlock_state(Node *n);

void node_set_state(Node *n, NodeState state);
