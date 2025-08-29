#pragma once

#include <raylib.h>

#include "defines.h"
#include "text.h"

typedef struct ButtonColors {
        Color normal;
        Color hovered;
        Color down;
        Color disabled;
        Color disabled_text;
        Color text;
} ButtonColors;

typedef enum ButtonState {
    BUTTON_STATE_NORMAL,
    BUTTON_STATE_HOVERED,
    BUTTON_STATE_DOWN,
    BUTTON_STATE_DISABLED
} ButtonState;

typedef struct Button {
        TextBox text;
        ButtonColors colors;
        ButtonState state;
} Button;

void button_create(Button *btn, Rectangle rect);

void button_destroy(Button *btn);

#define button_set_text_and_font(btn, txt, len, font) \
    text_box_set_text_and_font(&(btn)->text, txt, len, font)

void button_set_colors(Button *btn, ButtonColors colors);

// void button_set_color(Button *btn, Color btn_color, Color text_color);

bool button_update(Button *btn, Vector2 mpos);

void button_disable(Button *btn);

void button_enable(Button *btn);

void button_draw(Button *btn);
