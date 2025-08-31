#include "checkbox.h"

void check_box_create(CheckBox *cb, Rectangle rect) {
    cb->checked = false;
    cb->rect = rect;
    cb->inner_rect.width = rect.width * 0.8f;
    cb->inner_rect.height = rect.height * 0.8f;
    cb->inner_rect.x = rect.x + (rect.width * 0.1f);
    cb->inner_rect.y = rect.y + (rect.height * 0.1f);
    // cb->inner_rect.x = rect.x + (rect.width - cb->inner_rect.width) / 2;
    // cb->inner_rect.y = rect.y + (rect.height - cb->inner_rect.height) / 2;
    check_box_set_color(cb, WHITE);
}

void check_box_destroy(CheckBox *cb) {
    UNUSED(cb);
}

void check_box_draw(CheckBox *cb) {
    DrawRectangleRec(cb->rect, cb->color);
    if (cb->checked) {
        DrawRectangleRec(cb->inner_rect, cb->inner_color);
    }
}

i32 check_box_update(CheckBox *cb, Vector2 mpos, i32 handled) {
    if (CheckCollisionPointRec(mpos, cb->rect)
        && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            cb->checked = !cb->checked;
        }

        return handled;
    }

    return handled;
}

void check_box_set_color(CheckBox *cb, Color color) {
    cb->color = color;
    cb->inner_color = (Color){255 - cb->color.r, 255 - cb->color.g,
                              255 - cb->color.g, cb->color.a};
}

void check_box_set_checked(CheckBox *cb, bool checked) {
    cb->checked = checked;
}

bool check_box_get_checked(CheckBox *cb) {
    return cb->checked;
}
