#pragma once

#include "defines.h"
#include "node.h"
#include "tline.h"

typedef enum NfaState {
    NFA_STATE_OK = 0,
    NFA_STATE_EMPTY_ALPHABET,
    NFA_STATE_NO_INITIAL_STATE,
    NFA_STATE_NO_ACCEPTING_STATE,
    NFA_STATE_INPUT_INVALID,
    NFA_STATE_ACCEPTING_STATE_NOT_REACHABLE,
} NfaState;

Node **nfa_transition(Node *current_state, TLine *tlines, u64 tlines_length,
                      char input);

NfaState is_nfa_valid(Node *nodes, TLine *tlines, const char *alphabet);
