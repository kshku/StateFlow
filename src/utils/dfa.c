#include "dfa.h"

#include "darray.h"
#include "strops.h"

bool path_exists_to_accepting_state(Node **visited, TLine *tlines,
                                    u64 tlines_length, Node *current_state,
                                    const char *alphabet) {
    if (current_state->accepting_state) return true;

    u64 length = darray_get_size(visited);
    for (u64 i = 0; i < length; ++i)
        if (visited[i] == current_state) return false;
    darray_push(&visited, current_state);

    for (u64 i = 0; alphabet[i]; ++i) {
        if (path_exists_to_accepting_state(
                visited, tlines, tlines_length,
                dfa_transition(current_state, tlines, tlines_length,
                               alphabet[i]),
                alphabet))
            return true;
    }

    return false;
}

DfaState is_dfa_valid(Node *nodes, TLine *tlines, const char *alphabet) {
    if (!alphabet) return DFA_STATE_EMPTY_ALPHABET;

    Node *initial_state = NULL;
    bool accepting_state_exists = false;

    u64 nodes_length = darray_get_size(nodes);
    u64 tlines_length = darray_get_size(tlines);

    for (u64 i = 0; i < tlines_length; ++i)
        if (!all_chars_present(alphabet, tlines[i].inputs))
            return DFA_STATE_INPUT_INVALID;

    for (u64 i = 0; i < nodes_length; ++i) {
        if (nodes[i].initial_state) initial_state = &nodes[i];
        if (nodes[i].accepting_state) accepting_state_exists = true;
        u32 buf[128] = {0};
        for (u64 j = 0; j < tlines_length; ++j) {
            if (tlines[j].start == &nodes[i]) {
                for (u64 k = 0; k < tlines[j].len; ++k) {
                    buf[(int)tlines[j].inputs[k]]++;
                    if (buf[(int)tlines[j].inputs[k]] > 1)
                        return DFA_STATE_MULTIPLE_TRANSITIONS_DEFINED;
                }
            }
        }

        for (u64 i = 0; alphabet[i]; ++i) {
            if (!buf[(int)alphabet[i]]) {
                // TraceLog(LOG_INFO, "%c", alphabet[i]);
                return DFA_STATE_REQUIRE_ALL_INPUT_TRANSITIONS;
            }
        }
    }

    if (!initial_state) return DFA_STATE_NO_INITIAL_STATE;

    if (!accepting_state_exists) return DFA_STATE_NO_ACCEPTING_STATE;

    Node **visited = darray_create(Node *);
    if (!path_exists_to_accepting_state(visited, tlines, tlines_length,
                                        initial_state, alphabet)) {
        darray_destroy(visited);
        return DFA_STATE_ACCEPTING_STATE_NOT_REACHABLE;
    }
    darray_destroy(visited);

    return DFA_STATE_OK;
}

Node *dfa_transition(Node *current_state, TLine *tlines, u64 tlines_length,
                     char input) {
    char input_str[2] = {input, 0};
    for (u64 i = 0; i < tlines_length; ++i) {
        if (tlines[i].start == current_state
            && all_chars_present(tlines[i].inputs, input_str))
            return tlines[i].end;
    }

    return NULL;
}
