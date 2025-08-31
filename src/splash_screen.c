#include <raymath.h>

#include "stateflow.h"

static u32 logo_x, logo_y;
static u32 top_left, bottom_right;
static u16 frame_count;
static u8 state;
static u8 letters_count;
static RenderTexture2D target;
static Rectangle source, dest;
static float scale;
static Vector2 position;
static float alpha = 0.0f;

static Color bg = DARKGRAY;

static void splash_screen_update_transforms(void);

void splash_screen_load(GlobalState *gs) {
    target = LoadRenderTexture(800, 600);
    state = 0;
    frame_count = 0;
    letters_count = 0;
    top_left = bottom_right = 16;
    logo_x = (target.texture.width / 2) - 128;
    logo_y = (target.texture.height / 2) - 64;

    int width = MeasureText("StateFlow", 100);
    position =
        (Vector2){.x = (target.texture.width - width) / 2, .y = logo_y - 150};

    splash_screen_update_transforms();
}

void splash_screen_unload(GlobalState *gs) {
    UnloadRenderTexture(target);
    if (alpha < 1.0f) TraceLog(LOG_INFO, "Alpha is not 1");
}

ScreenChangeType splash_screen_update(GlobalState *gs) {
    splash_screen_update_transforms();

    float alpha_diff = 0.001;

    switch (state) {
        case 0:
            alpha_diff = 0.001;
            frame_count++;
            if (frame_count == 80) {
                frame_count = 0;
                state = 1;
            }
            break;
        case 1:
            alpha_diff = 0.002;
            top_left += 8;
            if (top_left >= 256) state = 2;
            break;
        case 2:
            alpha_diff = 0.004;
            bottom_right += 8;
            if (bottom_right >= 256) state = 3;
            break;
        case 3:
            alpha_diff = 0.008;
            bottom_right += 8;
            frame_count++;
            if (letters_count < 10 && frame_count / 12) {
                letters_count++;
                frame_count = 0;
            } else if (frame_count > 100) {
                gs->next_screen = &menu;
                return SCREEN_CHANGE;
            }
            break;
    }

    if (state > 0) {
        alpha += alpha_diff;
        alpha = CLAMP_MAX(alpha, 1.0);
    }

    return SCREEN_SAME;
}

void splash_screen_before_draw(GlobalState *gs) {
    BeginTextureMode(target);

    ClearBackground(bg);

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
                          256 - (2 * 16), bg);

            DrawText(TextSubtext("raylib", 0, letters_count),
                     (target.texture.width / 2) - 44,
                     (target.texture.height / 2) + 48 + 64, 50, BLACK);

            if (frame_count > 20)
                DrawText("powered by", logo_x, logo_y - 27, 20, LIGHTGRAY);
            break;
    }

    DrawText("StateFlow", position.x, position.y, 100,
             Fade((Color){50, 200, 255, 255}, alpha));

    EndTextureMode();
}

void splash_screen_draw(GlobalState *gs) {
    ClearBackground(bg);

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

static void splash_screen_update_transforms(void) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;
    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.6);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};

    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};

    dest.x = (width - dest.width) / 2;
    dest.y = (height - dest.height) / 2;
}

Screen splash_screen = (Screen){.load = splash_screen_load,
                                .unload = splash_screen_unload,
                                .draw = splash_screen_draw,
                                .before_draw = splash_screen_before_draw,
                                .update = splash_screen_update};
