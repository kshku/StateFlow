#pragma once

#include <raylib.h>

#include "defines.h"

typedef struct TextBox {
        Rectangle rect;
        Vector2 position;
        char *text;
        Color color;
        Font font;
        int font_size;
} TextBox;

void text_box_create(TextBox *tb, Rectangle rect);

void text_box_destroy(TextBox *tb);

// Text will be copied
void text_box_set_text_and_font(TextBox *tb, const char *text, u32 len,
                                Font font);

void text_box_set_color(TextBox *tb, Color color);

void text_box_draw(TextBox *tb);
