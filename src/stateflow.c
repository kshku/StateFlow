#include "stateflow.h"

#include <raymath.h>

#include "utils/darray.h"

typedef struct State {
        Screen *current_screen;
} State;

static State state = {0};
static GlobalState gs = {0};

static void stateflow_change_screen(void);

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

    state.current_screen = &splash_screen;
    state.current_screen->load(&gs);
}

void stateflow_shutdown(void) {
    state.current_screen->unload(&gs);

    darray_destroy(gs.nodes);
    darray_destroy(gs.tlines);

    UnloadFont(gs.font);

    CloseWindow();
}

void stateflow_run(void) {
    while (!WindowShouldClose()) {
        // stateflow_update_global_state();

        switch (state.current_screen->update(&gs)) {
            case SCREEN_CHANGE:
                stateflow_change_screen();
            case SCREEN_SAME:
            default:
                break;
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

// void stateflow_update_global_state(void) {
//     i32 width = GetScreenWidth();
//     i32 height = GetScreenHeight();
//     // i32 width = GetRenderWidth();
//     // i32 height = GetRenderHeight();

//     if (width < height) {
//         gs.virtual_width = 720;
//         gs.virtual_height = 1280;
//     } else {
//         gs.virtual_height = 720;
//         gs.virtual_width = 1280;
//     }

//     float scaleX = (float)width / gs.virtual_width;
//     float scaleY = (float)height / gs.virtual_height;
//     float scale = fminf(scaleX, scaleY);

//     Vector2 offset =
//         (Vector2){.x = (width - (gs.virtual_width * scale)) * 0.5,
//                   .y = (height - (gs.virtual_height * scale)) * 0.5};

//     gs.camera = (Camera2D){
//         .zoom = scale,
//         .target = (Vector2){0, 0},
//         .rotation = 0.0f,
//         .offset = offset
//     };

//     SetMouseOffset(-offset.x, -offset.y);
//     SetMouseScale(1 / scale, 1 / scale);
// }
