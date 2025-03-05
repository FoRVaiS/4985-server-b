// cppcheck-suppress-file unusedStructMember

#ifndef FSM_H
#define FSM_H

#include <unistd.h>

typedef enum
{
    START,
    REQUEST_HANDLER,
    HEADER_HANDLER,
    BODY_HANDLER,
    PROCESS_HANDLER,
    RESPONSE_HANDLER,
    ERROR_HANDLER,
    END
} fsm_state;

typedef int fsm_state_t;

typedef fsm_state_t (*fsm_state_func)(void *args);

struct fsm_transition
{
    fsm_state_t    from_id;
    fsm_state_t    to_id;
    fsm_state_func perform;
};

fsm_state_func fsm_transition(fsm_state_t from_id, fsm_state_t to_id, const struct fsm_transition transitions[], size_t transitions_size);

#endif    // FSM_H
