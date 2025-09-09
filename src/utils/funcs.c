#include "funcs.h"

#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/darray.h"
#include "utils/strops.h"

void draw_grid(Camera2D camera, float thick, float spacing, Color color) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();

    thick /= camera.zoom;

    Vector2 top_left = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottom_right = GetScreenToWorld2D((Vector2){width, height}, camera);

    float start_x = floorf(top_left.x / spacing) * spacing;
    float end_x = floorf(bottom_right.x / spacing) * spacing;
    float start_y = floorf(top_left.y / spacing) * spacing;
    float end_y = floorf(bottom_right.y / spacing) * spacing;

    for (float x = start_x; x <= end_x; x += spacing)
        DrawLineEx((Vector2){x, top_left.y}, (Vector2){x, bottom_right.y},
                   thick, color);

    for (float y = start_y; y <= end_y; y += spacing)
        DrawLineEx((Vector2){top_left.x, y}, (Vector2){bottom_right.x, y},
                   thick, color);
}

bool store_fsm_to_file(GlobalState *gs, const char *file_name) {
    FILE *file = fopen(file_name, "w");
    if (!file) return false;

    if (gs->fsm_type == FSM_TYPE_DFA) fprintf(file, "FSM_TYPE: %s\n", "DFA");
    else if (gs->fsm_type == FSM_TYPE_NFA)
        fprintf(file, "FSM_TYPE: %s\n", "NFA");

    u64 nodes_length = darray_get_size(gs->nodes);
    fprintf(file, "NODES:\n");
    for (u64 i = 0; i < nodes_length; ++i) {
        fprintf(file, "%s %u, %f %f, %u %u\n",
                gs->nodes[i].name ? gs->nodes[i].name : "NULL",
                gs->nodes[i].name_length, gs->nodes[i].center.x,
                gs->nodes[i].center.y, gs->nodes[i].initial_state,
                gs->nodes[i].accepting_state);
    }

    u64 tlines_length = darray_get_size(gs->tlines);
    fprintf(file, "TLINES:\n");
    for (u64 i = 0; i < tlines_length; ++i) {
        u64 start_idx = 0;
        u64 end_idx = 0;
        for (u64 j = 0; j < nodes_length; ++j) {
            if (&gs->nodes[j] == gs->tlines[i].start) start_idx = j;
            if (&gs->nodes[j] == gs->tlines[i].end) end_idx = j;
        }
        fprintf(file, "%s %u, %lu %lu", gs->tlines[i].inputs, gs->tlines[i].len,
                start_idx, end_idx);
    }

    fclose(file);

    return true;
}

bool load_fsm_from_file(GlobalState *gs, const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) return false;
    char buf[1024];

    if (fscanf(file, "FSM_TYPE: %s\n", buf) != 1) goto failed;

    if (!strcmp(buf, "DFA")) gs->fsm_type = FSM_TYPE_DFA;
    else if (!strcmp(buf, "NFA")) gs->fsm_type = FSM_TYPE_NFA;
    else goto failed;

    // TODO: Can we validate?
    fscanf(file, "NODES:\n");

    while (true) {
        int c = fgetc(file);
        if (c == EOF) goto failed;  // Unexpected EOF
        ungetc(c, file);

        if (c == 'T') break;  // TLINES

        u32 len;
        Vector2 center;
        Node node;
        if (fscanf(file, "%s %u, %f %f, %u %u\n", buf, &len, &center.x,
                   &center.y, &node.initial_state, &node.accepting_state)
            != 6)
            goto failed;
        node_create(&node, center);
        node_set_name(&node, buf, len);
        node_set_font(&node, gs->font, 32);
        node.editing = true;
        darray_push(&gs->nodes, node);
    }

    // TODO: Can we validate?
    fscanf(file, "TLINES:\n");

    u64 nodes_length = darray_get_size(gs->nodes);
    while (true) {
        int c = fgetc(file);
        // Well, we can have nothing at the tlines
        if (c == EOF) break;
        ungetc(c, file);

        u64 start_idx, end_idx;
        u32 len;
        if (fscanf(file, "%s %u, %lu %lu\n", buf, &len, &start_idx, &end_idx)
            != 4)
            goto failed;

        if (start_idx < 0 || start_idx >= nodes_length) goto failed;
        if (end_idx < 0 || end_idx >= nodes_length) goto failed;

        TLine tline;
        tline_create(&tline);
        tline_set_start_node(&tline, &gs->nodes[start_idx]);
        tline_set_end_node(&tline, &gs->nodes[end_idx]);
        tline_set_inputs(&tline, buf, len);
        tline_set_font(&tline, gs->font);
        tline.editing = true;

        darray_push(&gs->tlines, tline);
    }

    fclose(file);
    return true;
failed:
    fclose(file);
    return false;
}
