#pragma once

#include <raylib.h>

#include "defines.h"
#include "utils/tline.h"

typedef struct Screen Screen;

typedef enum ScreenChangeType {
    SCREEN_SAME = 0,
    SCREEN_CHANGE,
    SCREEN_FADE,
} ScreenChangeType;

typedef enum FSMType { FSM_TYPE_DFA, FSM_TYPE_NFA, FSM_TYPE_MAX } FSMType;

typedef struct GlobalState {
        Screen *next_screen;
        Font font;
        Node *nodes;  // Darray
        TLine *tlines;  // Darray
        FSMType fsm_type;
        char *alphabet;
        u64 alphabet_len;
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
extern Screen animation;
