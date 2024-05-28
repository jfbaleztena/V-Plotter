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

typedef struct vplot_s{
    state_t state;
    event_t event;
    cmd_s cmd;
}vplot_s;

void vplot_init();
void vplot_fsm(event_t new_event);
#endif