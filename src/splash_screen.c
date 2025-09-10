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

static Color bg;

static void splash_screen_update_transforms(void);

void splash_screen_load(GlobalState *gs) {
    UNUSED(gs);
    bg = DARKGRAY;
    target = LoadRenderTexture(1600, 1200);
    state = 0;
    frame_count = 0;
    letters_count = 0;
    top_left = bottom_right = 32;
    logo_x = (target.texture.width / 2) - 256;
    logo_y = (target.texture.height / 2) - 128;

    int width = MeasureText("StateFlow", 200);
    position =
        (Vector2){.x = (target.texture.width - width) / 2, .y = logo_y - 300};

    splash_screen_update_transforms();
}

void splash_screen_unload(GlobalState *gs) {
    UNUSED(gs);
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
            top_left += 16;
            if (top_left >= 512) state = 2;
            break;
        case 2:
            alpha_diff = 0.004;
            bottom_right += 16;
            if (bottom_right >= 512) state = 3;
            break;
        case 3:
            alpha_diff = 0.008;
            // bottom_right += 16;
            frame_count++;
            if (letters_count < 10 && frame_count / 12) {
                letters_count++;
                frame_count = 0;
            } else if (frame_count > 100) {
                gs->next_screen = &menu;
                return SCREEN_FADE;
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
    UNUSED(gs);
    BeginTextureMode(target);

    ClearBackground(bg);

    switch (state) {
        case 0:
            if ((frame_count / 10) % 2)
                DrawRectangle(logo_x, logo_y, 32, 32, BLACK);
            break;
        case 1:
            DrawRectangle(logo_x, logo_y, top_left, 32, BLACK);
            DrawRectangle(logo_x, logo_y, 32, top_left, BLACK);
            break;
        case 2:
            DrawRectangle(logo_x, logo_y, 512, 32, BLACK);
            DrawRectangle(logo_x, logo_y, 32, 512, BLACK);

            DrawRectangle(logo_x + 512 - 32, logo_y, 32, bottom_right, BLACK);
            DrawRectangle(logo_x, logo_y + 512 - 32, bottom_right, 32, BLACK);
            break;
        case 3:
            DrawRectangle(logo_x, logo_y, 512, 512, BLACK);
            DrawRectangle(logo_x + 32, logo_y + 32, 512 - (2 * 32),
                          512 - (2 * 32), bg);

            DrawText(TextSubtext("raylib", 0, letters_count),
                     (target.texture.width / 2) - 88,
                     (target.texture.height / 2) + 96 + 128, 100, BLACK);

            if (frame_count > 20)
                DrawText("powered by", logo_x, logo_y - 54, 40, LIGHTGRAY);
            break;
    }

    DrawText("StateFlow", position.x, position.y, 200,
             Fade((Color){50, 200, 255, 255}, alpha));

    EndTextureMode();
}

void splash_screen_draw(GlobalState *gs) {
    UNUSED(gs);
    ClearBackground(bg);

    DrawTexturePro(target.texture, source, dest, (Vector2){0}, 0.0f, WHITE);
}

static void splash_screen_update_transforms(void) {
    i32 width = GetScreenWidth();
    i32 height = GetScreenHeight();
    float scale_x = (float)width / target.texture.width;
    float scale_y = (float)height / target.texture.height;
    scale = fminf(scale_x, scale_y);

    scale = CLAMP_MIN(scale, 0.3);

    source = (Rectangle){0, 0, target.texture.width, -target.texture.height};

    dest = (Rectangle){.width = target.texture.width * scale,
                       .height = target.texture.height * scale};

    dest.x = (width - dest.width) / 2;
    dest.y = (height - dest.height) / 2;
}

Screen splash_screen = {.load = splash_screen_load,
                        .unload = splash_screen_unload,
                        .draw = splash_screen_draw,
                        .before_draw = splash_screen_before_draw,
                        .update = splash_screen_update};
