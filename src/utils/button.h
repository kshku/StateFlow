#pragma once

#include <raylib.h>

#include "defines.h"
#include "text.h"

// typedef struct ButtonColors {
//         Color normal;
//         Color hovered;
//         Color clicked;
//         Color text;
// } ButtonColors;

typedef struct Button {
        TextBox text;
        // ButtonColors colors;
        Color color;
} Button;

void button_create(Button *btn, Rectangle rect);

void button_destroy(Button *btn);

#define button_set_text_and_font(btn, txt, len, font) \
    text_box_set_text_and_font(&(btn)->text, txt, len, font)

// void button_set_colors(Button *btn, ButtonColors colors);

void button_set_color(Button *btn, Color btn_color, Color text_color);

bool button_update(Button *btn, Vector2 mpos);

void button_draw(Button *btn);
