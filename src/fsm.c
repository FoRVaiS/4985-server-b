#include "fsm.h"
#include <stdio.h>

fsm_state_func fsm_transition(fsm_state_t from_id, fsm_state_t to_id, const struct fsm_transition transitions[], size_t transitions_size)
{
    fsm_state_func transition_func;

    transition_func = NULL;

    for(size_t i = 0; i < transitions_size / sizeof(transitions[0]); i++)
    {
        if(transitions[i].from_id == from_id && transitions[i].to_id == to_id)
        {
            transition_func = transitions[i].perform;
            break;
        }
    }

    return transition_func;
}
