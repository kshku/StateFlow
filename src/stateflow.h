#pragma once

#include <raylib.h>

#include "defines.h"

typedef struct Screen Screen;

typedef enum ScreenChangeType {
    SCREEN_SAME = 0,
    SCREEN_CHANGE,
} ScreenChangeType;

typedef struct GlobalState {
        Screen *next_screen;
        // i32 virtual_width;
        // i32 virtual_height;
        // Camera2D camera;
} GlobalState;

struct Screen {
        void (*load)(GlobalState *gs);
        void (*unload)(GlobalState *gs);
        ScreenChangeType (*update)(GlobalState *gs);
        void (*draw)(GlobalState *gs);
        void (*before_draw)(GlobalState *gs);
};

extern Screen splash_screen;
extern Screen menu;
extern Screen editor;
