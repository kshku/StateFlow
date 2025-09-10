#include "stateflow.h"

#include <raymath.h>
#include <stdlib.h>

#include "utils/darray.h"

typedef struct State {
        Screen *current_screen;
        u64 frame_count;
        bool transitioning;
        bool fade_out;
        float alpha;
} State;

static State state = {0};
static GlobalState gs = {0};

static void stateflow_change_screen(void);
static void stateflow_fade_screen(void);
static void stateflow_update_fading(void);
static void stateflow_draw_fade(void);

void stateflow_initialize(void) {
    InitWindow(800, 600, STATEFLOW_NAME);

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    SetTargetFPS(60);

    gs.font =
        // LoadFont("assets/JetBrainsMonoNerdFont-Medium.ttf");
        LoadFontEx("assets/FiraCodeNerdFontMono-Bold.ttf", 144, NULL, 0);
    gs.fsm_type = FSM_TYPE_MAX;
    gs.nodes = darray_create(Node);
    gs.tlines = darray_create(TLine);
    gs.alphabet = NULL;
    gs.alphabet_len = 0;

    state.current_screen = &splash_screen;
    state.transitioning = false;
    state.alpha = 0.0;
    state.frame_count = 0;

    // ##########################################
    // For the development phase
    // gs.alphabet = malloc(2 * sizeof(char));
    // gs.alphabet[0] = 'a';
    // gs.alphabet[1] = 0;
    // gs.alphabet_len = 1;
    // NodeColors node_colors = {.text = GREEN,
    //                           .normal = BLUE,
    //                           .hovered = DARKBLUE,
    //                           .down = VIOLET,
    //                           .highlighted = YELLOW};
    // TLineColors tline_colors = {.text = GREEN,
    //                             .normal = GREEN,
    //                             .down = BLACK,
    //                             .hovered = DARKBLUE,
    //                             .highlighted = YELLOW};
    // gs.fsm_type = FSM_TYPE_DFA;
    // Node node;
    // node_create(&node, (Vector2){500, 300});
    // node_set_colors(&node, node_colors);
    // node_set_font(&node, gs.font, 32);
    // node_set_name(&node, "a", 1);
    // node.initial_state = true;
    // node.accepting_state = true;
    // darray_push(&gs.nodes, node);

    // node_create(&node, (Vector2){700, 500});
    // node_set_colors(&node, node_colors);
    // node_set_font(&node, gs.font, 32);
    // node_set_name(&node, "a", 1);
    // node.initial_state = false;
    // node.accepting_state = false;
    // darray_push(&gs.nodes, node);

    // TLine tline;
    // tline_create(&tline);
    // tline_set_colors(&tline, tline_colors);
    // tline_set_start_node(&tline, &gs.nodes[0]);
    // tline_set_end_node(&tline, &gs.nodes[1]);
    // tline_set_inputs(&tline, "a", 1);
    // tline_set_font(&tline, gs.font);
    // darray_push(&gs.tlines, tline);

    // tline_create(&tline);
    // tline_set_colors(&tline, tline_colors);
    // tline_set_start_node(&tline, &gs.nodes[1]);
    // tline_set_end_node(&tline, &gs.nodes[0]);
    // tline_set_inputs(&tline, "a", 1);
    // tline_set_font(&tline, gs.font);
    // darray_push(&gs.tlines, tline);

    // state.current_screen = &animation;
    // ##################################################

    state.current_screen->load(&gs);
}

void stateflow_shutdown(void) {
    state.current_screen->unload(&gs);

    darray_destroy(gs.nodes);
    darray_destroy(gs.tlines);
    free(gs.alphabet);
    gs.alphabet = NULL;

    UnloadFont(gs.font);

    CloseWindow();
}

void stateflow_run(void) {
    while (!WindowShouldClose()) {
        if (state.transitioning) {
            stateflow_update_fading();
        } else {
            switch (state.current_screen->update(&gs)) {
                case SCREEN_FADE:
                    stateflow_fade_screen();
                    break;
                case SCREEN_CHANGE:
                    stateflow_change_screen();
                case SCREEN_SAME:
                default:
                    break;
            }
        }

        if (state.current_screen->before_draw)
            state.current_screen->before_draw(&gs);

        BeginDrawing();
        state.current_screen->draw(&gs);
        // DrawText(
        //     TextFormat("(%d, %d)\n(%d, %d)", GetScreenWidth(),
        //                GetScreenHeight(), GetRenderWidth(),
        //                GetRenderHeight()),
        //     50, 50, 50, GREEN);
        if (state.transitioning) stateflow_draw_fade();

        EndDrawing();
    }
}

int main(void) {
    stateflow_initialize();
    stateflow_run();
    stateflow_shutdown();
}

static void stateflow_change_screen(void) {
    if (!gs.next_screen) {
        // TraceLog(LOG_ERROR, "Next screen is not given!");
        return;
    }

    state.current_screen->unload(&gs);
    state.current_screen = gs.next_screen;
    gs.next_screen = NULL;
    state.current_screen->load(&gs);
}

static void stateflow_fade_screen(void) {
    state.transitioning = true;
    state.alpha = 0.0;
    state.fade_out = false;
}

static void stateflow_update_fading(void) {
    if (!state.fade_out) {
        state.alpha += 0.02f;
        if (state.alpha > 1.01f) {
            state.alpha = 1.0f;

            state.current_screen->unload(&gs);
            state.current_screen = gs.next_screen;
            gs.next_screen = NULL;
            state.current_screen->load(&gs);

            state.fade_out = true;
        }
    } else {
        state.alpha -= 0.02f;
        if (state.alpha < -0.01f) {
            state.alpha = 0.0f;
            state.fade_out = false;
            state.transitioning = false;
        }
    }
}

static void stateflow_draw_fade(void) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Fade(BLACK, state.alpha));
}
