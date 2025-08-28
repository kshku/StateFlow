#include "button.h"

void button_create(Button *btn, Rectangle rect) {
    // btn->colors.normal = WHITE;
    // btn->colors.hovered = WHITE;
    // btn->colors.clicked = WHITE;
    btn->color = WHITE;
    text_box_create(&btn->text, rect);
}

void button_destroy(Button *btn) {
    text_box_destroy(&btn->text);
}

// void button_set_colors(Button *btn, ButtonColors colors) {
//     btn->colors = colors;
//     text_box_set_color(&btn->text, colors.text);
// }

void button_set_color(Button *btn, Color btn_color, Color text_color) {
    btn->color = btn_color;
    text_box_set_color(&btn->text, text_color);
}

void button_draw(Button *btn) {
    // DrawRectangleRec(btn->text.rect, btn->colors.normal);
    DrawRectangleRec(btn->text.rect, btn->color);
    text_box_draw(&btn->text);
}

bool button_update(Button *btn, Vector2 mpos) {
    // Returns true if clicked
    return CheckCollisionPointRec(mpos, btn->text.rect)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
