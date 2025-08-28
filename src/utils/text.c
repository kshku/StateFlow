#include "text.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void text_box_create(TextBox *tb, Rectangle rect) {
    tb->rect = rect;
    tb->font_size = rect.height;
    tb->color = BLACK;
    tb->text = NULL;
}

void text_box_set_text_and_font(TextBox *tb, const char *text, u32 len,
                                Font font) {
    char *new_text = (char *)realloc(tb->text, len + 1);
    if (!new_text) return;
    strncpy(new_text, text, len);
    new_text[len] = 0;
    tb->text = new_text;
    tb->font = font;

    Vector2 size = MeasureTextEx(tb->font, tb->text, tb->font_size, 1);

    float scale_x = tb->rect.width / size.x;
    float scale_y = tb->rect.height / size.y;
    float scale = fminf(scale_x, scale_y);

    tb->font_size = tb->rect.height * scale;
    tb->position =
        (Vector2){.x = tb->rect.x + ((tb->rect.width - (size.x * scale)) / 2),
                  .y = tb->rect.y + ((tb->rect.height - (size.y * scale)) / 2)};
}

void text_box_set_color(TextBox *tb, Color color) {
    tb->color = color;
}

void text_box_draw(TextBox *tb) {
    DrawTextEx(tb->font, tb->text, tb->position, tb->font_size, 1, tb->color);
}

void text_box_destroy(TextBox *tb) {
    free(tb->text);
}
