#pragma once

#include <raylib.h>

#include "defines.h"
#include "stateflow.h"

void draw_grid(Camera2D camera, float thick, float spacing, Color color);

bool store_fsm_to_file(GlobalState *gs, const char *file_name);

bool load_fsm_from_file(GlobalState *gs, const char *file_name);
