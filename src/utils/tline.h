#pragma once

#include <raylib.h>

#include "defines.h"
#include "node.h"

typedef struct TLine {
        Color color;
        Node *start;
        Node *end;
        char *inputs;
        u32 len;
        Vector2 *points;
} TLine;

void tline_create(TLine *tl, const char *inputs, u32 len);

void tline_destroy(TLine *tl);

void tline_set_start_node(TLine *tl, Node *n);

void tline_set_end_node(TLine *tl, Node *n);

void tline_set_inputs(TLine *tl, const char *inputs, u32 len);

void tline_set_colors(TLine *tl, Color color);

const char *tline_get_inputs(TLine *tl, u32 *len);

i32 tline_update(TLine *tl, Vector2 mpos, Node *nodes, i32 handled);

void tline_draw(TLine *tl);

void tline_add_point(TLine *tl, Vector2 point);
