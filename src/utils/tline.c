#include "tline.h"

#include <raymath.h>
#include <stdlib.h>

void tline_create(TLine *tl, const char *inputs, u32 len) {
    tline_set_colors(tl, YELLOW);
    tline_set_start_node(tl, NULL);
    tline_set_end_node(tl, NULL);
    tl->inputs = NULL;
    tl->len = 0;
    tl->pressed = false;
    tl->selected = false;
}

void tline_destroy(TLine *tl) {
    free(tl->inputs);
}

void tline_set_colors(TLine *tl, Color color) {
    tl->color = color;
}

void tline_set_start_node(TLine *tl, Node *n) {
    tl->start = n;
}

void tline_set_end_node(TLine *tl, Node *n) {
    tl->end = n;
}

void tline_set_inputs(TLine *tl, const char *inputs, u32 len) {
    char *new_inputs = (char *)realloc(tl->inputs, (len + 1) * sizeof(char));
    if (!new_inputs) return;
    for (u32 i = 0; i < len; ++i) new_inputs[i] = inputs[i];
    new_inputs[len] = 0;
    tl->inputs = new_inputs;
    tl->len = len;
}

const char *tline_get_inputs(TLine *tl, u32 *len) {
    if (len) *len = tl->len;
    return tl->inputs;
}

i32 tline_update(TLine *tl, Vector2 mpos, i32 handled) {
    if (CheckCollisionPointLine(mpos, tl->start->center, tl->end->center, 5.0f)
        && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION);

        if (IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) return handled;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            tl->pressed = true;
            handled = MARK_INPUT_HANDLED(handled, MOUSE_BUTTON_LEFT);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && tl->pressed) {
            handled = MARK_INPUT_HANDLED(handled, MOUSE_BUTTON_LEFT);
            tl->pressed = false;
            tl->selected = true;
        }
        return handled;
    }

    tl->pressed = false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON))
        tl->selected = false;

    return handled;
}

void tline_draw(TLine *tl) {
    if (tl->start && tl->end) {
        Vector2 center = Vector2Lerp(tl->start->center, tl->end->center, 0.53);
        Vector2 dir = Vector2Normalize(
            Vector2Subtract(tl->end->center, tl->start->center));
        Vector2 perp = {-dir.y, dir.x};
        Vector2 points[3] = {Vector2Add(center, Vector2Scale(perp, 10)),
                             Vector2Add(center, Vector2Scale(dir, 20)),
                             Vector2Add(center, Vector2Scale(perp, -10))};

        Vector2 text_pos = Vector2Add(center, Vector2Scale(perp, 11));
        // if (dir.x < 0) text_pos = Vector2Add(center, Vector2Scale(perp,
        // -11));

        // DrawLineBezier(tl->start->center, tl->end->center, 2.0f,
        //                tl->selected ? BLACK : tl->color);
        DrawLineEx(tl->start->center, tl->end->center, 3.0f,
                   tl->selected ? BLACK : tl->color);
        DrawTriangle(points[0], points[1], points[2], BLACK);
        DrawTextEx(GetFontDefault(), tl->inputs, text_pos, 32, 1.0f, BLACK);
    }
}

void tline_append_input(TLine *tl, const char *input, u32 len) {
    char *new_input =
        (char *)realloc(tl->inputs, (tl->len + len + 1) * sizeof(char));
    if (!new_input) return;
    for (i32 i = tl->len; i < tl->len + len; ++i)
        tl->inputs[i - tl->len] = input[i];
    tl->len += len;
    tl->inputs[tl->len] = 0;
}
