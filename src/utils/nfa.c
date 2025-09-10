#include "nfa.h"

#include "darray.h"
#include "strops.h"

static bool path_exists_to_accepting_state(Node **visited, TLine *tlines,
                                           u64 tlines_length,
                                           Node *current_state,
                                           const char *alphabet) {
    if (current_state->accepting_state) return true;

    u64 length = darray_get_size(visited);
    for (u64 i = 0; i < length; ++i)
        if (visited[i] == current_state) return false;
    darray_push(&visited, current_state);

    for (u64 i = 0; alphabet[i]; ++i) {
        Node **states = darray_create(Node *);
        states = nfa_transition(current_state, states, tlines, tlines_length,
                                alphabet[i]);
        u64 length = darray_get_size(states);
        for (u64 i = 0; i < length; ++i) {
            if (path_exists_to_accepting_state(visited, tlines, tlines_length,
                                               states[i], alphabet)) {
                darray_destroy(states);
                return true;
            }
        }
        darray_destroy(states);
    }

    return false;
}

NfaState is_nfa_valid(Node *nodes, TLine *tlines, const char *alphabet) {
    if (!alphabet) return NFA_STATE_EMPTY_ALPHABET;

    Node *initial_state = NULL;
    bool accepting_state_exists = false;

    u64 nodes_length = darray_get_size(nodes);
    u64 tlines_length = darray_get_size(tlines);

    for (u64 i = 0; i < tlines_length; ++i)
        if (!all_chars_present(alphabet, tlines[i].inputs))
            return NFA_STATE_INPUT_INVALID;

    for (u64 i = 0; i < nodes_length; ++i) {
        if (nodes[i].initial_state) initial_state = &nodes[i];
        if (nodes[i].accepting_state) accepting_state_exists = true;
        if (initial_state && accepting_state_exists) break;
    }

    if (!initial_state) return NFA_STATE_NO_INITIAL_STATE;
    if (!accepting_state_exists) return NFA_STATE_NO_ACCEPTING_STATE;

    Node **visited = darray_create(Node *);
    if (!path_exists_to_accepting_state(visited, tlines, tlines_length,
                                        initial_state, alphabet)) {
        darray_destroy(visited);
        return NFA_STATE_ACCEPTING_STATE_NOT_REACHABLE;
    }
    darray_destroy(visited);

    return NFA_STATE_OK;
}

Node **nfa_transition(Node *current_state, Node **states /*returned*/,
                      TLine *tlines, u64 tlines_length, char input) {
    char input_str[2] = {input, 0};
    for (u64 i = 0; i < tlines_length; ++i) {
        if (tlines[i].start == current_state
            && all_chars_present(tlines[i].inputs, input_str)) {
            u64 length = darray_get_size(states);
            for (u64 j = 0; j < length; ++j)
                if (states[j] == tlines[i].end) continue;
            darray_push(&states, tlines[i].end);
        }
    }

    return states;
}
