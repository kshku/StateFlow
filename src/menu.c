#include <math.h>

#include "stateflow.h"

void menu_load(GlobalState *gs) {
}

void menu_unload(GlobalState *gs) {
}

ScreenChangeType menu_update(GlobalState *gs) {
    return SCREEN_SAME;
}

void menu_draw(GlobalState *gs) {
}

Screen menu = (Screen){.load = menu_load,
                       .unload = menu_unload,
                       .draw = menu_draw,
                       .update = menu_update};
