#pragma once

#include <raylib.h>

#include "defines.h"

typedef struct InputBoxColors {
        Color text;
        Color box;
} InputBoxColors;

typedef struct InputBox {
        Rectangle rect;
        InputBoxColors colors;
        Vector2 position;
        Font font;
        float font_size;
        bool focused;
        bool pressed;
        u32 max_len;
        u32 index;
        char *text;
        u32 chars_to_show;
} InputBox;

void input_box_create(InputBox *ib, Rectangle rect, u32 max_len);

void input_box_destroy(InputBox *ib);

void input_box_set_colors(InputBox *ib, InputBoxColors colors);

void input_box_set_font(InputBox *ib, Font font);

void input_box_draw(InputBox *ib);

bool input_box_update(InputBox *ib, Vector2 mpos);
