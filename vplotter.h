#ifndef ___VPLOTTER_H___
#define ___VPLOTTER_H___

#include "hpgl.h"

typedef enum event_t{
    NO_EVENT,
    PA_REQ,
    PU_REQ,
    PD_REQ,
    PR_REQ,
    IN_REQ,
    IP_REQ,
    SC_REQ,
    SP_REQ,
    NEW_STRING,
    VALID_STRING,
    INVALID_STRING,
    VALID_ARG,
    INVALID_ARG,
    OK,
    FAIL
}event_t;

typedef enum state_t{
    IDLE,
    INITIALIZE,
    HPGL_PARSE,
    PA,
    PR,
    PU,
    PD,
    BOARD_CONF,
    EM_STOP
}state_t;

#define MAX_EVENT_QUEUE 10
typedef struct vplot_s{
    state_t state;
    event_t event_queue[10];
    uint8_t last_event_index;
    cmd_s cmd;
    bool initialized;
}vplot_s;

void vplot_init();
void vplot_fsm(event_t new_event);
void vplot_run();

bool vplot_event_queue(event_t event);
event_t vplot_event_dequeue();
bool vplot_event_is_empty();
#endif