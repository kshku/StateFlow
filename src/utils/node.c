#include "node.h"

#include <raymath.h>
#include <stdlib.h>
#include <string.h>

#define NODE_MINIMUM_RADIUS 50.0f

static i32 node_update_editing(Node *n, Vector2 mpos, Vector2 delta,
                               i32 handled);

static i32 node_update_animating(Node *n, i32 handled);

void node_create(Node *n, Vector2 center) {
    n->center = center;
    n->initial_state = n->accepting_state = false;
    n->position = n->center;
    n->font = GetFontDefault();
    n->font_size = 30;
    n->name = NULL;
    n->name_length = 0;
    n->state = NODE_STATE_NORMAL;
    n->colors = (NodeColors){.normal = BLUE,
                             .hovered = DARKBLUE,
                             .text = GREEN,
                             .down = VIOLET,
                             .highlighted = YELLOW};
    n->radius = NODE_MINIMUM_RADIUS;
    n->editing = true;
    n->moving = false;
    // n->locked = false;
    n->selected = false;
    n->pressed = false;
}

void node_destroy(Node *n) {
    free(n->name);
}

void node_set_name(Node *n, const char *name, u32 len) {
    char *new_name = (char *)realloc(n->name, (len + 1) * sizeof(char));
    if (!new_name) {
        n->radius = NODE_MINIMUM_RADIUS;
        return;
    }
    strncpy(new_name, name, len);
    new_name[len] = 0;
    n->name_length = len;
    n->name = new_name;

    Vector2 size = MeasureTextEx(n->font, n->name, n->font_size, 1.0f);
    n->radius = CLAMP_MIN((size.x / 2.0f) + 10.0f, NODE_MINIMUM_RADIUS);

    n->position = (Vector2){
        .x = n->center.x - (size.x / 2.0f),
        .y = n->center.y - (size.y / 2.0f),
    };
}

void node_set_font(Node *n, Font font, float font_size) {
    n->font = font;
    n->font_size = font_size;

    if (n->name) {
        Vector2 size = MeasureTextEx(n->font, n->name, n->font_size, 1.0f);
        n->radius = size.x / 2;
    }
}

void node_set_colors(Node *n, NodeColors colors) {
    n->colors = colors;
}

void node_draw(Node *n) {
    Color color;

    switch (n->state) {
        case NODE_STATE_HIGHLIGHTED:
            color = n->colors.highlighted;
            break;
        case NODE_STATE_HOVERED:
            color = n->colors.hovered;
            break;
        case NODE_STATE_DOWN:
            color = n->colors.down;
            break;
        case NODE_STATE_NORMAL:
        default:
            color = n->colors.normal;
            break;
    }

    if (n->accepting_state) {
        DrawCircleV(n->center, n->radius, BLACK);
        DrawCircleV(n->center, n->radius * 0.9, color);
    } else {
        DrawCircleV(n->center, n->radius, color);
    }
    DrawTextEx(n->font, n->name, n->position, n->font_size, 1.0f,
               n->colors.text);

    if (n->initial_state) {
        // DrawSplineLinear(n->points, 3, 3.0f, BLACK);
        Vector2 points[3] = {

            {n->center.x - (n->radius * 1.5f),
             n->center.y - (n->radius * 0.5f)             },
            {         n->center.x - n->radius, n->center.y},
            {n->center.x - (n->radius * 1.5f),
             n->center.y + (n->radius * 0.5f)             }
        };
        DrawTriangle(points[2], points[1], points[0], BLACK);
    }
}

i32 node_update(Node *n, Vector2 mpos, Vector2 delta, i32 handled) {
    return n->editing ? node_update_editing(n, mpos, delta, handled)
                      : node_update_animating(n, handled);
}

// bool node_update(Node *n, Vector2 mpos) {
//     // TEMP
//     if (n->state == NODE_STATE_HIGHLIGHTED) return false;

//     if (n->locked) {
//         n->state = NODE_STATE_SELECTED;
//         return CheckCollisionPointCircle(mpos, n->center, n->radius)
//             && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
//     }

//     if (CheckCollisionPointCircle(mpos, n->center, n->radius)) {
//         if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
//             // Vector2 delta = GetMouseDelta();
//             // if (delta.x != 0 || delta.y != 0) {
//             //     n->moving = true;
//             //     n->center = Vector2Add(delta, n->center);
//             //     n->position = Vector2Add(delta, n->position);
//             // }
//             n->state = NODE_STATE_SELECTED;
//         }

//         if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
//             // if (n->moving) {
//             //     n->moving = false;
//             //     n->state = NODE_STATE_HOVERED;
//             //     return false;
//             // } else {
//             n->state = NODE_STATE_SELECTED;
//             return true;
//             // }
//         } else {
//             n->state = NODE_STATE_HOVERED;
//         }
//         return false;
//     }

//     n->state = NODE_STATE_NORMAL;

//     return false;
// }

// void node_lock_selected(Node *n) {
//     n->locked = true;
// }

// void node_unlock_selected(Node *n) {
//     n->locked = false;
// }

// void node_toggle_highlight(Node *n) {
//     n->state = n->state == NODE_STATE_HIGHLIGHTED ? NODE_STATE_NORMAL
//                                                   : NODE_STATE_HIGHLIGHTED;
// }

static i32 node_update_editing(Node *n, Vector2 mpos, Vector2 delta,
                               i32 handled) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (delta.x != 0 || delta.y != 0)
        && n->selected && n->pressed
        && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
        handled = MARK_INPUT_HANDLED(handled,
                                     INPUT_LEFT_BUTTON | INPUT_MOUSE_POSITION);
        n->state = NODE_STATE_DOWN;
        n->moving = true;
        n->center = Vector2Add(delta, n->center);
        n->position = Vector2Add(delta, n->position);
        return handled;
    }

    if (CheckCollisionPointCircle(mpos, n->center, n->radius)
        && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
        n->state = NODE_STATE_HOVERED;
        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            n->pressed = true;
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            n->state = NODE_STATE_DOWN;
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && n->pressed) {
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            if (n->moving) n->moving = false;
            else {
                n->state = NODE_STATE_DOWN;
                n->selected = true;
                n->pressed = false;
            }
        }

        if (n->selected) n->state = NODE_STATE_DOWN;

        return handled;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON))
        n->selected = false;

    n->pressed = false;
    n->moving = false;

    n->state = NODE_STATE_NORMAL;

    if (n->selected) n->state = NODE_STATE_DOWN;

    return handled;
}

static i32 node_update_animating(Node *n, i32 handled) {
    n->state = NODE_STATE_NORMAL;

    return handled;
}

// void node_lock_state(Node *n, NodeState state) {
//     n->locked = true;
//     n->state = state;
// }

// void node_unlock_state(Node *n) {
//     n->locked = false;
// }

// void node_set_state(Node *n, NodeState state) {
//     n->state = state;
// }
