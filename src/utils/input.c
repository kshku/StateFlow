#include "input.h"

#include <stdlib.h>

static void take_input_text(InputBox *ib);

void input_box_create(InputBox *ib, Rectangle rect, u32 max_len) {
    ib->rect = rect;
    ib->font_size = rect.height;
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
    ib->frame_counter = 0;
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

    if (ib->focused && ib->index < ib->max_len - 1) {
        ib->text[ib->index + 1] = 0;
        if (((ib->frame_counter / 20) % 2) == 0) ib->text[ib->index] = '_';
        else ib->text[ib->index] = 0;
    } else {
        ib->text[ib->index] = 0;
    }

    DrawTextEx(ib->font, &ib->text[idx], ib->position, ib->font_size, 1.0f,
               ib->colors.text);
}

i32 input_box_update(InputBox *ib, Vector2 mpos, i32 handled) {
    ib->frame_counter++;
    if (ib->focused && !IS_INPUT_HANDLED(handled, INPUT_KEYSTROKES)) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        TraceLog(LOG_INFO, "Waiting for input!");
        take_input_text(ib);
        handled = MARK_INPUT_HANDLED(handled, INPUT_KEYSTROKES);
        // MARK_INPUT_HANDLED(handled, INPUT_KEYSTROKES | INPUT_MOUSE_POSITION
        //                                 | INPUT_LEFT_BUTTON);
    } else {
        SetMouseCursor(MOUSE_CURSOR_ARROW);
    }

    if (CheckCollisionPointRec(mpos, ib->rect)
        && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
        TraceLog(LOG_INFO, "Collided");
        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION);

        if (IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) return handled;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ib->pressed = true;
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && ib->pressed) {
            // TraceLog(LOG_INFO, "Focused!");
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            ib->pressed = false;
            ib->focused = true;
        }

        return handled;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
        ib->focused = false;
        TraceLog(LOG_INFO, "Unfocused!");
    }

    ib->pressed = false;

    return handled;
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
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            ib->index = 0;
        if (ib->index > 0) ib->index--;
        ib->text[ib->index] = 0;
    }
}

const char *input_box_get_text(InputBox *ib, u32 *len) {
    if (len) *len = ib->index;
    return ib->text;
}

void input_box_set_text(InputBox *ib, const char *text, u32 len) {
    for (ib->index = 0; ib->index < len && ib->index < ib->max_len; ++ib->index)
        ib->text[ib->index] = text[ib->index];
}

void input_box_append_text(InputBox *ib, const char *text, u32 len) {
    for (u32 i = 0; i < len && ib->index < ib->max_len; ++ib->index, ++i)
        ib->text[ib->index] = text[i];
}

void input_box_append_text_at(InputBox *ib, const char *text, u32 len,
                              u32 idx) {
    u32 i;
    for (i = 0, ib->index = idx; i < len && ib->index < ib->max_len;
         ++i, ++ib->index)
        ib->text[ib->index] = text[i];
}
