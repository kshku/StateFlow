#include "strops.h"

bool all_chars_present(const char *alphabets, const char *inputs) {
    u32 buf[128] = {0};
    for (u64 i = 0; alphabets[i]; ++i) buf[alphabets[i]]++;
    for (u64 i = 0; inputs[i]; ++i)
        if (!buf[inputs[i]]) return false;
    return true;
}
