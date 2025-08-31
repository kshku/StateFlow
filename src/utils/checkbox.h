#pragma once

#include <raylib.h>

#include "defines.h"

typedef struct CheckBox {
        Color color;
        Color inner_color;
        Rectangle rect;
        Rectangle inner_rect;
        bool checked;
} CheckBox;

void check_box_create(CheckBox *cb, Rectangle rect);

void check_box_destroy(CheckBox *cb);

void check_box_draw(CheckBox *cb);

i32 check_box_update(CheckBox *cb, Vector2 mpos, i32 handled);

void check_box_set_color(CheckBox *cb, Color color);

void check_box_set_checked(CheckBox *cb, bool checked);

bool check_box_get_checked(CheckBox *cb);
