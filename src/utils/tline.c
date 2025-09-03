#include "tline.h"

#include <raymath.h>
#include <stdlib.h>

bool check_collision_point_bezier_cubic(Vector2 point, Vector2 p0, Vector2 p1,
                                        Vector2 p2, Vector2 p3, float thickness,
                                        u32 segments);

void tline_create(TLine *tl) {
    tline_set_colors(tl, YELLOW);
    tline_set_start_node(tl, NULL);
    tline_set_end_node(tl, NULL);
    tline_set_font(tl, GetFontDefault());
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

void tline_set_font(TLine *tl, Font font) {
    tl->font = font;
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
    TraceLog(LOG_INFO, "%s", inputs);
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
    if (!tl->start || !tl->end) return handled;

    bool collided = false;

    if (tl->start != tl->end) {
        collided = CheckCollisionPointLine(mpos, tl->start->center,
                                           tl->end->center, 5.0f);
    } else {
        tl->points[0] = (Vector2){tl->start->center.x, tl->start->center.y};
        tl->points[1] =
            (Vector2){tl->start->center.x + 80.0f,
                      tl->start->center.y - tl->start->radius * 3.0f};
        tl->points[2] =
            (Vector2){tl->start->center.x - 80.0f,
                      tl->start->center.y - tl->start->radius * 3.0f};
        tl->points[3] = (Vector2){tl->start->center.x, tl->start->center.y};

        collided = check_collision_point_bezier_cubic(
            mpos, tl->points[0], tl->points[1], tl->points[2], tl->points[3],
            3.0f, 50);
    }

    if (collided && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
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
        if (tl->start != tl->end) {
            Vector2 center =
                Vector2Lerp(tl->start->center, tl->end->center, 0.53);
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
            DrawTriangle(points[0], points[1], points[2],
                         tl->selected ? BLACK : tl->color);
            DrawTextEx(tl->font, tl->inputs, text_pos, 32, 1.0f,
                       tl->selected ? BLACK : tl->color);
        } else {
            // Self loop
            DrawSplineBezierCubic(tl->points, 4, 3.0f,
                                  tl->selected ? BLACK : tl->color);
            DrawTextEx(
                tl->font, tl->inputs,
                (Vector2){tl->start->center.x - 30.0f,
                          tl->start->center.y - tl->start->radius * 3.0f},
                32, 1.0f, tl->selected ? BLACK : tl->color);
        }
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

bool check_collision_point_bezier_cubic(Vector2 mpos, Vector2 p0, Vector2 p1,
                                        Vector2 p2, Vector2 p3, float thickness,
                                        u32 segments) {
    Vector2 prev = p0;

    for (u32 i = 1; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;

        // Bezier cubic formula
        Vector2 pos = {u * u * u * p0.x + 3 * u * u * t * p1.x
                           + 3 * u * t * t * p2.x + t * t * t * p3.x,
                       u * u * u * p0.y + 3 * u * u * t * p1.y
                           + 3 * u * t * t * p2.y + t * t * t * p3.y};

        // Check click against segment
        if (CheckCollisionPointLine(mpos, prev, pos, thickness)) return true;

        prev = pos;
    }
    return false;
}
