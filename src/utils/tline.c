#include "tline.h"

#include <stdlib.h>

#include "darray.h"

void tline_create(TLine *tl, const char *inputs, u32 len) {
    tline_set_colors(tl, YELLOW);
    tline_set_start_node(tl, NULL);
    tline_set_end_node(tl, NULL);
    tline_set_inputs(tl, NULL, 0);
    tl->points = darray_create(Vector2);
}

void tline_destroy(TLine *tl) {
    free(tl->inputs);
    darray_destroy(tl->points);
}

void tline_set_colors(TLine *tl, Color color) {
    tl->color = color;
}

void tline_set_start_node(TLine *tl, Node *n) {
    tl->start = n;
    if (n) darray_push_at(&tl->points, 0, n->center);
}

void tline_set_end_node(TLine *tl, Node *n) {
    tl->end = n;
    if (n) darray_push(&tl->points, n->center);
}

void tline_add_point(TLine *tl, Vector2 point) {
    if (!tl->end) darray_push(tl->points, point);
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

i32 tline_update(TLine *tl, Vector2 mpos, Node *nodes, i32 handled) {
    u64 len = darray_get_size(nodes);
    if (!tl->start) {
        for (u32 i = 0; i < len; ++i) {
            handled = node_update(&nodes[i], mpos, (Vector2){0, 0}, handled);
            if (nodes[i].selected) tline_set_start_node(tl, &nodes[i]);
        }

        return handled;
    }

    if (!tl->end) {
        for (u32 i = 0; i < len; ++i) {
            handled = node_update(&nodes[i], mpos, (Vector2){0, 0}, handled);
            if (nodes[i].selected) tline_set_end_node(tl, &nodes[i]);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            darray_push(&tl->points, mpos);
        }

        if (!tl->end) darray_push(&tl->points, mpos);
    }
}

void tline_draw(TLine *tl) {
    u64 len = darray_get_size(tl->points);
    if (len > 1) {
        DrawSplineLinear(tl->points, len, 1.0f, tl->color);
    }
}
