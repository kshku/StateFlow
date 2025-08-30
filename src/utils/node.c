#include "node.h"

#include <raymath.h>
#include <stdlib.h>
#include <string.h>

static NodeStatus node_update_editing(Node *n, Vector2 mpos, Vector2 delta);

static NodeStatus node_update_animating(Node *n);

void node_create(Node *n, Vector2 center) {
    n->center = center;
    n->position = n->center;
    n->font = GetFontDefault();
    n->font_size = 30;
    n->name = NULL;
    n->state = NODE_STATE_NORMAL;
    n->colors = (NodeColors){.normal = WHITE,
                             .hovered = WHITE,
                             .text = WHITE,
                             .down = WHITE,
                             .highlighted = WHITE};
    n->radius = 50.0f;
    n->editing = true;
    n->moving = false;
    n->locked = false;
    n->selected = false;
    n->pressed = false;
}

void node_destroy(Node *n) {
    free(n->name);
}

void node_set_name(Node *n, char *name, u32 len) {
    char *new_name = (char *)realloc(n->name, (len + 1) * sizeof(char));
    if (!new_name) return;
    strncpy(new_name, name, len);
    new_name[len] = 0;
    n->name = new_name;

    Vector2 size = MeasureTextEx(n->font, n->name, n->font_size, 1.0f);
    n->radius = size.x / 2;

    n->position = (Vector2){
        .x = n->center.x - (size.x / 2),
        .y = n->center.y - (size.y / 2),
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

    DrawCircleV(n->center, n->radius, color);
    DrawTextEx(n->font, n->name, n->position, n->font_size, 1.0f,
               n->colors.text);
}

NodeStatus node_update(Node *n, Vector2 mpos, Vector2 delta) {
    return n->editing ? node_update_editing(n, mpos, delta)
                      : node_update_animating(n);
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

static NodeStatus node_update_editing(Node *n, Vector2 mpos, Vector2 delta) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (delta.x != 0 || delta.y != 0)
        && n->selected) {
        n->moving = true;
        n->center = Vector2Add(delta, n->center);
        n->position = Vector2Add(delta, n->position);
        return NODE_MOVING;
    }

    if (CheckCollisionPointCircle(mpos, n->center, n->radius)) {
        if (!n->locked) n->state = NODE_STATE_HOVERED;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) n->pressed = true;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (!n->locked) n->state = NODE_STATE_DOWN;
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && n->pressed) {
            if (n->moving) n->moving = false;
            else {
                n->selected = true;
                n->pressed = false;
                return NODE_CLICKED;
            }
        }

        return NODE_HOVERED;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) n->selected = false;

    n->pressed = false;
    n->moving = false;

    if (!n->locked) n->state = NODE_STATE_NORMAL;

    return NODE_NOT_AFFECTED;
}

static NodeStatus node_update_animating(Node *n) {
}

void node_lock_state(Node *n, NodeState state) {
    n->locked = true;
    n->state = state;
}

void node_unlock_state(Node *n) {
    n->locked = false;
}

void node_set_state(Node *n, NodeState state) {
    n->state = state;
}
