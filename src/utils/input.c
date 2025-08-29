#include "input.h"

void input_box_create(InputBox *ib, Rectangle rect, u32 max_len) {
    ib->rect = rect;
    ib->font = GetFontDefault();
    ib->font_size = max_len;
}

void input_box_set_font(InputBox *ib, Font font) {
    ib->font = font;
}

void input_box_destroy(InputBox *ib) {
}

void input_box_draw(InputBox *ib) {
}

void input_box_update(InputBox *ib, Vector2 mpos) {
}
