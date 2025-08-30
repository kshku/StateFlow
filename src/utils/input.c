#include "input.h"

#include <stdlib.h>

static void take_input_text(InputBox *ib);

void input_box_create(InputBox *ib, Rectangle rect, u32 max_len) {
    ib->rect = rect;
    ib->font_size = rect.height * 0.8f;
    ib->focused = false;
    ib->pressed = false;
    ib->index = 0;
    ib->max_len = max_len;
    ib->text = (char *)calloc((max_len + 1), sizeof(char));
    if (!ib->text) {
        ib->max_len = 0;
        return;
    }

    ib->text[max_len] = 0;

    input_box_set_font(ib, GetFontDefault());
    ib->colors = (InputBoxColors){.box = WHITE, .text = BLACK};
}

void input_box_set_font(InputBox *ib, Font font) {
    ib->font = font;

    Vector2 size = MeasureTextEx(ib->font, "A", ib->font_size, 1.0f);
    ib->chars_to_show = (int)ib->rect.width / size.x;
    TraceLog(LOG_INFO, "chars_to_show = %d", ib->chars_to_show);

    ib->position = (Vector2){
        .x = ib->rect.x
           + ((ib->rect.width - ((ib->chars_to_show - 1) * size.x)) / 2),
        .y = ib->rect.y + ((ib->rect.height - size.y) / 2)};
}

void input_box_destroy(InputBox *ib) {
    free(ib->text);
}

void input_box_set_colors(InputBox *ib, InputBoxColors colors) {
    ib->colors = colors;
}

void input_box_draw(InputBox *ib) {
    DrawRectangleRec(ib->rect, ib->colors.box);
    u32 idx = 0;
    if (ib->index >= ib->chars_to_show) {
        idx = ib->index - ib->chars_to_show;
    }
    DrawTextEx(ib->font, &ib->text[idx], ib->position, ib->font_size, 1.0f,
               ib->colors.text);
}

bool input_box_update(InputBox *ib, Vector2 mpos) {
    if (ib->focused) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        TraceLog(LOG_INFO, "Waiting for input!");
        take_input_text(ib);
    } else {
        SetMouseCursor(MOUSE_CURSOR_ARROW);
    }

    if (CheckCollisionPointRec(mpos, ib->rect)) {
        TraceLog(LOG_INFO, "Collided");
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ib->pressed = true;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && ib->pressed) {
            TraceLog(LOG_INFO, "Focused!");
            ib->pressed = false;
            ib->focused = true;
        }

        return ib->focused;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ib->focused = false;

    ib->pressed = false;

    return ib->focused;
}

static void take_input_text(InputBox *ib) {
    for (int key = GetCharPressed(); key > 0; key = GetCharPressed()) {
        // Key validation
        if ((key >= 32 && key <= 126) && ib->index < ib->max_len) {
            ib->text[ib->index++] = key;
            ib->text[ib->index] = 0;
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (ib->index > 0) ib->index--;
        ib->text[ib->index] = 0;
    }
}
