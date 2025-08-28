#include <math.h>

#include "stateflow.h"

// void splash_screen_setup_virtual_resolution(void);

// void splash_screen_setup_virtual_resolution(void) {
//     int width = GetScreenWidth();
//     int height = GetScreenHeight();
//     // int width = GetRenderWidth();
//     // int height = GetRenderHeight();

//     if (width < height) {
//         virtual_width = 450;
//         virtual_height = 800;
//     } else {
//         virtual_width = 800;
//         virtual_height = 450;
//     }

//     float scaleX = (float)width / virtual_width;
//     float scaleY = (float)height / virtual_height;
//     float scale = fminf(scaleX, scaleY);

//     camera = (Camera2D){
//         .zoom = scale,
//         .rotation = 0,
//         .target = (Vector2){0},
//         .offset = {.x = (width - (virtual_width * scale)) * 0.5,
//                             .y = (height - (virtual_height * scale)) * 0.5}
//     };
// }

static u32 logo_x, logo_y;
static u32 top_left, bottom_right;
static u16 frame_count;
static u8 state;
static u8 letters_count;
static RenderTexture2D target;

void splash_screen_load(GlobalState *gs) {
    target = LoadRenderTexture(800, 450);
    state = 0;
    frame_count = 0;
    letters_count = 0;
    top_left = bottom_right = 16;
    logo_x = (target.texture.width / 2) - 128;
    logo_y = (target.texture.height / 2) - 128;
}

void splash_screen_unload(GlobalState *gs) {
    UnloadRenderTexture(target);
}

ScreenChangeType splash_screen_update(GlobalState *gs) {
    // splash_screen_setup_virtual_resolution();
    logo_x = (target.texture.width / 2) - 128;
    logo_y = (target.texture.height / 2) - 128;

    switch (state) {
        case 0:
            frame_count++;
            if (frame_count == 80) {
                frame_count = 0;
                state = 1;
            }
            break;
        case 1:
            top_left += 8;
            if (top_left >= 256) state = 2;
            break;
        case 2:
            bottom_right += 8;
            if (bottom_right >= 256) state = 3;
            break;
        case 3:
            frame_count++;
            if (letters_count < 10 && frame_count / 12) {
                letters_count++;
                frame_count = 0;
            } else if (frame_count > 200) {
                gs->next_screen = &menu;
                return SCREEN_CHANGE;
            }
            break;
    }
    return SCREEN_SAME;
}

void splash_screen_before_draw(GlobalState *gs) {
    BeginTextureMode(target);

    ClearBackground(RAYWHITE);

    switch (state) {
        case 0:
            if ((frame_count / 10) % 2)
                DrawRectangle(logo_x, logo_y, 16, 16, BLACK);
            break;
        case 1:
            DrawRectangle(logo_x, logo_y, top_left, 16, BLACK);
            DrawRectangle(logo_x, logo_y, 16, top_left, BLACK);
            break;
        case 2:
            DrawRectangle(logo_x, logo_y, 256, 16, BLACK);
            DrawRectangle(logo_x, logo_y, 16, 256, BLACK);

            DrawRectangle(logo_x + 256 - 16, logo_y, 16, bottom_right, BLACK);
            DrawRectangle(logo_x, logo_y + 256 - 16, bottom_right, 16, BLACK);
            break;
        case 3:
            DrawRectangle(logo_x, logo_y, 256, 256, BLACK);
            DrawRectangle(logo_x + 16, logo_y + 16, 256 - (2 * 16),
                          256 - (2 * 16), RAYWHITE);

            DrawText(TextSubtext("raylib", 0, letters_count),
                     (target.texture.width / 2) - 44,
                     (target.texture.height / 2) + 48, 50, BLACK);

            if (frame_count > 20)
                DrawText("powered by", logo_x, logo_y - 27, 20, DARKGRAY);
            break;
    }

    EndTextureMode();
}

void splash_screen_draw(GlobalState *gs) {
    ClearBackground(RAYWHITE);

    int width = GetScreenWidth();
    int height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;
    float scale = fminf(scale_x, scale_y);

    Rectangle source = {0, 0, target.texture.width, -target.texture.height};
    Rectangle dest = {.width = target.texture.width,
                      .height = target.texture.height};
    dest.x = (width - dest.width) / 2;
    dest.y = (height - dest.height) / 2;
    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

Screen splash_screen = (Screen){.load = splash_screen_load,
                                .unload = splash_screen_unload,
                                .draw = splash_screen_draw,
                                .before_draw = splash_screen_before_draw,
                                .update = splash_screen_update};
