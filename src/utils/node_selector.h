#pragma once

#include <raylib.h>

#include "defines.h"
#include "node.h"

typedef struct NodeSelector {
        Rectangle rect;
        Vector2 position;
        bool pressed;
        bool selected;
        Node *node;
        Font font;
        float font_size;
        u32 chars_to_show;
} NodeSelector;

void node_selector_create(NodeSelector *ns, Rectangle rect);

void node_selector_destroy(NodeSelector *ns);

i32 node_selector_update(NodeSelector *ns, Node *nodes, Vector2 mpos,
                         Vector2 world_mpos, i32 handled);

void node_selector_draw(NodeSelector *ns);

void node_selector_set_font(NodeSelector *ns, Font font);
