#pragma once

#include <raylib.h>

#include "defines.h"

typedef struct InputBox {
        Rectangle rect;
        Font font;
        float font_size;
} InputBox;

void input_box_create(InputBox *ib, Rectangle rect, u32 max_len);

void input_box_destroy(InputBox *ib);

void input_box_set_font(InputBox *ib, Font font);

void input_box_draw(InputBox *ib);

void input_box_update(InputBox *ib, Vector2 mpos);
