#include "button.h"

void button_create(Button *btn, Rectangle rect) {
    btn->colors.normal = WHITE;
    btn->colors.hovered = WHITE;
    btn->colors.down = WHITE;
    btn->colors.disabled = WHITE;
    text_box_create(&btn->text, rect);
    btn->state = BUTTON_STATE_NORMAL;
}

void button_destroy(Button *btn) {
    text_box_destroy(&btn->text);
}

void button_set_colors(Button *btn, ButtonColors colors) {
    btn->colors = colors;
    text_box_set_color(&btn->text, colors.text);
}

// void button_set_color(Button *btn, Color btn_color, Color text_color) {
//     btn->color = btn_color;
//     text_box_set_color(&btn->text, text_color);
// }

void button_draw(Button *btn) {
    Color color;
    switch (btn->state) {
        case BUTTON_STATE_DOWN:
            color = btn->colors.down;
            break;
        case BUTTON_STATE_HOVERED:
            color = btn->colors.hovered;
            break;
        case BUTTON_STATE_DISABLED:
            color = btn->colors.disabled;
            break;
        case BUTTON_STATE_NORMAL:
        default:
            color = btn->colors.normal;
            break;
    }
    DrawRectangleRec(btn->text.rect, color);
    // DrawRectangleRec(btn->text.rect, btn->color);
    text_box_draw(&btn->text);
}

bool button_update(Button *btn, Vector2 mpos) {
    if (btn->state == BUTTON_STATE_DISABLED) return false;
    // Returns true if clicked
    if (CheckCollisionPointRec(mpos, btn->text.rect)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            btn->state = BUTTON_STATE_DOWN;
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            btn->state = BUTTON_STATE_HOVERED;
            return true;
        } else {
            btn->state = BUTTON_STATE_HOVERED;
        }

        return false;
    }

    btn->state = BUTTON_STATE_NORMAL;

    return false;
}

void button_disable(Button *btn) {
    btn->state = BUTTON_STATE_DISABLED;
    text_box_set_color(&btn->text, btn->colors.disabled_text);
}

void button_enable(Button *btn) {
    btn->state = BUTTON_STATE_NORMAL;
    text_box_set_color(&btn->text, btn->colors.text);
}
