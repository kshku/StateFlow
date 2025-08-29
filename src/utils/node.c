#include "node.h"

#include <stdlib.h>
#include <string.h>

void node_create(Node *n, Vector2 center) {
    n->center = center;
    n->position = n->center;
    n->font = GetFontDefault();
    n->font_size = 30;
    n->name = NULL;
    n->state = NODE_STATE_NORMAL;
    n->colors =
        (NodeColors){.normal = WHITE, .hovered = WHITE, .selected = WHITE};
    n->radius = 1.0f;
    n->locked = false;
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

    Vector2 size = MeasureTextEx(n->font, n->name, n->font_size, 1.0f);
    n->radius = size.x / 2;
}

void node_set_colors(Node *n, NodeColors colors) {
    n->colors = colors;
}

void node_draw(Node *n) {
    Color color;

    switch (n->state) {
        case NODE_STATE_HOVERED:
            color = n->colors.hovered;
            break;
        case NODE_STATE_SELECTED:
            color = n->colors.selected;
            break;
        case NODE_STATE_NORMAL:
        default:
            color = n->colors.normal;
            break;
    }

    DrawCircleV(n->center, n->radius, color);
    DrawTextEx(n->font, n->name, n->position, n->font_size, 1.0f,
               n->colors.font);
}

bool node_update(Node *n, Vector2 mpos) {
    if (n->locked) {
        n->state = NODE_STATE_SELECTED;
        return CheckCollisionPointCircle(mpos, n->center, n->radius)
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    }

    if (CheckCollisionPointCircle(mpos, n->center, n->radius)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            n->state = NODE_STATE_SELECTED;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            n->state = NODE_STATE_SELECTED;
            return true;
        }

        else
            n->state = NODE_STATE_HOVERED;

        return false;
    }

    n->state = NODE_STATE_NORMAL;

    return false;
}

void node_lock_selected(Node *n) {
    n->locked = true;
}

void node_unlock_selected(Node *n) {
    n->locked = false;
}
