#pragma once

#include "defines.h"
#include "node.h"
#include "tline.h"

typedef enum DfaState {
    DFA_STATE_OK = 0,
    DFA_STATE_EMPTY_ALPHABET,
    DFA_STATE_NO_INITIAL_STATE,
    DFA_STATE_NO_ACCEPTING_STATE,
    DFA_STATE_INPUT_INVALID,
    DFA_STATE_MULTIPLE_TRANSITIONS_DEFINED,
    DFA_STATE_REQUIRE_ALL_INPUT_TRANSITIONS,
    DFA_STATE_ACCEPTING_STATE_NOT_REACHABLE,
} DfaState;

Node *dfa_transition(Node *current_state, TLine *tlines, u64 tlines_length,
                     char input);

DfaState is_dfa_valid(Node *nodes, TLine *tlines, const char *alphabet);
