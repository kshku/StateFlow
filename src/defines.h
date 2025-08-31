#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIX(a, b) ((a) < (b) ? (a) : (b))

#define CLAMP(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define CLAMP_MIN(x, a) ((x) < (a) ? (a) : (x))
#define CLAMP_MAX(x, a) ((x) > (a) ? (a) : (x))

typedef enum Input {
    INPUT_NONE = 0,
    INPUT_KEYSTROKES = 1 << 0,
    INPUT_LEFT_BUTTON = 1 << 1,
    INPUT_RIGHT_BUTTON = 1 << 2,
    INPUT_MIDDLE_BUTTON = 1 << 3,
    INPUT_MOUSE_POSITION = 1 << 4,
    INPUT_MOUSE_WHEEL = 1 << 5,
} Input;

#define IS_INPUT_HANDLED(handled, input) ((handled) & (input))
#define MARK_INPUT_HANDLED(handled, input) ((handled) | (input))
