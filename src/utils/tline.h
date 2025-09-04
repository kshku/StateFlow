#pragma once

#include <raylib.h>

#include "defines.h"
#include "node.h"

typedef struct TLine {
        Color color;
        // Bezier curve
        Vector2 points[4];
        Node *start;
        Node *end;
        Font font;
        char *inputs;
        u32 len;
        bool pressed;
        bool selected;
} TLine;

void tline_create(TLine *tl);

void tline_destroy(TLine *tl);

void tline_set_start_node(TLine *tl, Node *n);

void tline_set_end_node(TLine *tl, Node *n);

void tline_set_inputs(TLine *tl, const char *inputs, u32 len);

void tline_set_colors(TLine *tl, Color color);

void tline_set_font(TLine *tl, Font font);

const char *tline_get_inputs(TLine *tl, u32 *len);

i32 tline_update(TLine *tl, Vector2 mpos, i32 handled);

void tline_draw(TLine *tl);

void tline_append_inputs(TLine *tl, const char *input, u32 len);
