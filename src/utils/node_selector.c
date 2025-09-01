#include "node_selector.h"

#include "darray.h"

void node_selector_create(NodeSelector *ns, Rectangle rect) {
    ns->node = NULL;
    ns->pressed = false;
    ns->selected = false;
    ns->rect = rect;
    ns->font_size = rect.height;
    ns->font = GetFontDefault();
    node_selector_set_font(ns, GetFontDefault());
}

void node_selector_destroy(NodeSelector *ns) {
    UNUSED(ns);
}

i32 node_selector_update(NodeSelector *ns, Node *nodes, Vector2 mpos,
                         Vector2 world_mpos, i32 handled) {
    if (ns->selected) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        u64 length = darray_get_size(nodes);
        for (i32 i = length - 1; i > -1; --i) {
            handled =
                node_update(&nodes[i], world_mpos, (Vector2){0, 0}, handled);
            if (nodes[i].selected) {
                nodes[i].selected = false;
                ns->node = &nodes[i];
                ns->selected = false;
                SetMouseCursor(MOUSE_CURSOR_ARROW);
            }
        }
    }

    if (CheckCollisionPointRec(mpos, ns->rect)
        && !IS_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION)) {
        handled = MARK_INPUT_HANDLED(handled, INPUT_MOUSE_POSITION);

        if (IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) return handled;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            ns->pressed = true;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && ns->pressed) {
            handled = MARK_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON);
            ns->pressed = false;
            ns->selected = true;
        }
        return handled;
    }

    ns->pressed = false;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && !IS_INPUT_HANDLED(handled, INPUT_LEFT_BUTTON)) {
        ns->selected = false;
        SetMouseCursor(MOUSE_CURSOR_ARROW);
    }

    return handled;
}

void node_selector_draw(NodeSelector *ns) {
    DrawRectangleRec(ns->rect, WHITE);
    if (ns->node) {
        u32 idx = 0;
        if (ns->node->name_length >= ns->chars_to_show)
            idx = ns->node->name_length - ns->chars_to_show;
        DrawTextEx(ns->font, &ns->node->name[idx], ns->position, ns->font_size,
                   1.0f, BLACK);
    }
}

void node_selector_set_font(NodeSelector *ns, Font font) {
    ns->font = font;

    Vector2 size = MeasureTextEx(font, "A", ns->font_size, 1.0f);
    ns->chars_to_show = (u32)ns->rect.width / size.x;
    ns->position = (Vector2){
        .x = ns->rect.x
           + ((ns->rect.width - ((ns->chars_to_show - 1) * size.x)) / 2),
        .y = ns->rect.y + ((ns->rect.height - size.y) / 2)};
}
