#pragma once

#include <raylib.h>

#include "defines.h"

typedef enum NodeState {
    NODE_STATE_NORMAL,
    NODE_STATE_SELECTED,
    NODE_STATE_HOVERED
} NodeState;

typedef struct NodeColors {
        Color normal;
        Color selected;
        Color hovered;
        Color font;
} NodeColors;

typedef struct Node {
        char *name;
        Font font;
        float font_size;
        Vector2 center;
        Vector2 position;
        NodeColors colors;
        float radius;
        NodeState state;
        bool locked;
} Node;

void node_create(Node *n, Vector2 center);

void node_destroy(Node *n);

void node_set_name(Node *n, char *name, u32 len);

void node_set_font(Node *n, Font font, float font_size);

void node_set_colors(Node *n, NodeColors colors);

void node_draw(Node *n);

void node_lock_selected(Node *n);
void node_unlock_selected(Node *n);

bool node_update(Node *n, Vector2 mpos);
