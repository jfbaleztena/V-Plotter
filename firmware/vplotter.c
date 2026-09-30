#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vplotter.h"
#include "servo.h"
#include "custom_serial.h"
#include "hpgl.h"
#include "stepper_wraper.h"
#include "draw_tools.h"

// #define CONFIG_DEBUG_STATES
// #define CONFIG_DEBUG_EVENTS
#ifdef CONFIG_DEBUG_EVENTS
char* event_to_string(event_t event);
#endif

void clear_event_queue();
bool vplot_event_queue_is_empty();
void vplot_event_remove_last();

vplot_s vplot;

void vplot_init(){
    vplot.state = IDLE;
    clear_event_queue();
    vplot_event_queue_is_empty();
    vplot.initialized = false;
}

void clear_event_queue(){
    memset(vplot.event_queue, NO_EVENT, sizeof(vplot.event_queue));
    vplot.last_event_index = 0;
}

bool vplot_event_queue(event_t event){
    if(vplot.last_event_index > MAX_EVENT_QUEUE - 1) return false;
#ifdef CONFIG_DEBUG_EVENTS
    printf("Event queued: %s(%d)\r\n", event_to_string(event), event);
#endif
    vplot.event_queue[vplot.last_event_index] = event;
    vplot.last_event_index++;
    return true;
}

event_t vplot_event_dequeue(){
    if(vplot.last_event_index == 0) return NO_EVENT;
    vplot.last_event_index--;
    event_t buff = vplot.event_queue[vplot.last_event_index];
    vplot.event_queue[vplot.last_event_index] = NO_EVENT;
    return buff;
}

bool vplot_event_queue_is_empty(){
    return vplot.last_event_index == 0 ? true : false;
}

void vplot_run(){
    if(vplot_event_queue_is_empty() == true) return;
    vplot_fsm(vplot_event_dequeue());
}

void vplot_mechanical_init(){
    if(vplot.initialized == true){
        vplot_event_queue(EVT_FAIL);
        return;
    }
    if(servo_init() == true){
        if(stepper_wraper_init() == true){
            vplot_event_queue(INIT_OK);
            vplot.initialized = true;
            return;
        }
    }
    vplot_event_queue(EVT_FAIL);
}

void vplot_fsm(event_t new_event){
#ifdef CONFIG_DEBUG_EVENTS
    printf("New event: %s(%d)\r\n", event_to_string(new_event), new_event);
#endif
    switch(vplot.state){
        case IDLE:
            if(new_event == NEW_STRING){
                vplot.state = HPGL_PARSE;
                parse_command(rx_line);
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: IDLE -> PARSE\r\n");
            }else{
                printf("FSM: IDLE ERR, UNK EVENT\r\n");
#endif
            }
            break;

        case HPGL_PARSE:
            switch(new_event){
                case INVALID_STRING:
                    vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> IDLE\r\n");
#endif
                    nack();
                    break;
                case PO_REQ: // custom command for moving to origin
                    vplot.state = PO;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> ORG\r\n");
#endif
                    move_absolute(INITIAL_X, INITIAL_Y);
                    break;
                case PA_REQ:
                    vplot.state = PA;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> PA\r\n");
#endif
                    validate_arg();
                    break;
                case PR_REQ:
                    vplot.state = PR;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> PR\r\n");
#endif
                    validate_arg();
                    break;
                case PU_REQ:
                    vplot.state = PU;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> PU\r\n");
#endif
                    validate_arg();
                    break;
                case PD_REQ:
                    vplot.state = PD;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> PD\r\n");
#endif
                    validate_arg();
                    break;
                case IP_REQ:
                    vplot.state = BOARD_CONF;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> BOARD_CONF(IP)\r\n");
#endif
                    validate_arg();
                    break;
                case IN_REQ:
                    vplot.state = INITIALIZE;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> INITIALIZE\r\n");
#endif
                    vplot_mechanical_init();
                    break;
                case SC_REQ:
                    vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> IDLE(SC)\r\n");
#endif
                    ack();
                    break;
                case SP_REQ:
                    vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                    printf("FSM: PARSE -> IDLE(SP)\r\n");
#endif
                    ack();
                    break;
                default: 
                    vplot.state = IDLE;
                    // printf("FSM: PARSE ERR, UNK EVENT %d\r\n", new_event);
                    nack(); 
                    break;
            }
            break;

        case INITIALIZE:
            if(new_event == INIT_OK){
                vplot.state = INITIALIZE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: INITIALIZE -> INITIALIZE\r\n");
#endif
                pen_up();
            }else if(new_event == EVT_OK){
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: INITIALIZE -> IDLE\r\n");
#endif
                ack();
            }else if(new_event == EVT_FAIL){
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: INITIALIZE -> IDLE\r\n");
#endif
                nack();
            }else{
                vplot.state = IDLE;
                // printf("FSM: INITIALIZE ERR, UNK EVENT %d\r\n", new_event);
                nack();
            }
            break;

        case PO:
            if(new_event == EVT_OK){
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: ORG -> IDLE\r\n");
#endif
                ack();
            }else if(new_event == EVT_FAIL){
                vplot.state = EM_STOP;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: ORG -> EM_STOP\r\n");
#endif
            }else{
                vplot.state = IDLE;
                // printf("FSM: ORG ERR, UNK EVENT %d\r\n", new_event);
                nack();
            }
            break;

        case PA:
            if(new_event == VALID_ARG || new_event == EVT_OK){
                vplot.state = PA;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PA -> PA\r\n");
#endif
                if(vplot.cmd.arg_used < vplot.cmd.arg_count/2){
                    move_absolute(vplot.cmd.arg[2*vplot.cmd.arg_used], vplot.cmd.arg[2*vplot.cmd.arg_used+1]);
                    vplot.cmd.arg_used++;
                }else{
                    vplot_event_queue(MOV_END);
                }
            }else if(new_event == EVT_FAIL){
                vplot.state = EM_STOP;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PA -> EM_STOP\r\n");
#endif
            }else{
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PA -> IDLE\r\n");
#endif
                new_event == MOV_END ? ack() : nack(); // if != MOV_END, then it's an invalid argument
            }
            break;

        case PR:
            if(new_event == VALID_ARG || new_event == EVT_OK){
                vplot.state = PR;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PR -> PR\r\n");
#endif
                if(vplot.cmd.arg_used < vplot.cmd.arg_count/2){
                    move_relative(vplot.cmd.arg[2*vplot.cmd.arg_used], vplot.cmd.arg[2*vplot.cmd.arg_used+1]);
                    vplot.cmd.arg_used++;
                }else{
                    vplot_event_queue(MOV_END);
                }
            }else if(new_event == EVT_FAIL){
                vplot.state = EM_STOP;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PR -> EM_STOP\r\n");
#endif
            }else{
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PR -> IDLE\r\n");
#endif
                new_event == MOV_END ? ack() : nack(); // if != MOV_END, then it's an invalid argument
            }
            break;

        case PU:
            if(new_event == VALID_ARG){
                vplot.state = PU;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PU -> PU\r\n");
#endif
                pen_up();
            }else if(new_event == EVT_OK){
                vplot.state = PU;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PU -> PU\r\n");
#endif
                if(vplot.cmd.arg_used < vplot.cmd.arg_count/2){
                    move_absolute(vplot.cmd.arg[2*vplot.cmd.arg_used], vplot.cmd.arg[2*vplot.cmd.arg_used+1]);
                    vplot.cmd.arg_used++;
                }else{
                    vplot_event_queue(MOV_END);
                }
            }else if(new_event == EVT_FAIL){
                vplot.state = EM_STOP;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PU -> EM_STOP\r\n");
#endif
            }else{
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PU -> IDLE\r\n");
#endif
                new_event == MOV_END ? ack() : nack(); // if != MOV_END, then it's an invalid argument
            }
            break;

        case PD:
            if(new_event == VALID_ARG){
                vplot.state = PD;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PD -> PD\r\n");
#endif
                pen_down();
            }else if(new_event == EVT_OK){
                vplot.state = PD;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PD -> PD\r\n");
#endif
                if(vplot.cmd.arg_used < vplot.cmd.arg_count/2){
                    move_absolute(vplot.cmd.arg[2*vplot.cmd.arg_used], vplot.cmd.arg[2*vplot.cmd.arg_used+1]);
                    vplot.cmd.arg_used++;
                }else{
                    vplot_event_queue(MOV_END);
                }
            }else if(new_event == EVT_FAIL){
                vplot.state = EM_STOP;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PD -> EM_STOP\r\n");
#endif
            }else{
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: PD -> IDLE\r\n");
#endif
                new_event == MOV_END ? ack() : nack(); // if != MOV_END, then it's an invalid argument
            }
            break;

        case BOARD_CONF:
            if(new_event == EVT_OK){
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: BOARD_CONF -> IDLE\r\n");
#endif
                get_board_coordinates();
                ack();
            }else if(new_event == EVT_FAIL || new_event == INVALID_ARG){
                vplot.state = IDLE;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: BOARD_CONF -> IDLE\r\n");
#endif
                nack();
            }else if(new_event == VALID_ARG){
                vplot.state = BOARD_CONF;
#ifdef CONFIG_DEBUG_STATES
                printf("FSM: BOARD_CONF -> BOARD_CONF\r\n");
#endif
                set_board_coodinates(vplot.cmd.arg[0], vplot.cmd.arg[1], vplot.cmd.arg[2], vplot.cmd.arg[3]);
            }else{
                vplot.state = IDLE;
                nack();
            }
            break;
            
        case EM_STOP:
#ifdef CONFIG_DEBUG_STATES
            printf("FSM: EM_STOP -> IDLE\r\n");
#endif
            vplot.state = IDLE;
            nack();
            break;

        default: break; 
    }
}

#ifdef CONFIG_DEBUG_STATES
char* event_to_string(event_t event){
    switch(event){
        case EVT_OK: return "EVT_OK";
        case EVT_FAIL: return "EVT_FAIL";
        case INVALID_ARG: return "INVALID_ARG";
        case VALID_ARG: return "VALID_ARG";
        case NEW_STRING: return "NEW_STRING";
        case INIT_OK: return "INIT_OK";
        case MOV_END: return "MOV_END";
        case PA_REQ: return "PA_REQ";
        case PU_REQ: return "PU_REQ";
        case PD_REQ: return "PD_REQ";
        case PR_REQ: return "PR_REQ";
        case IP_REQ: return "IP_REQ";
        case IN_REQ: return "IN_REQ";
        case SC_REQ: return "SC_REQ";
        case SP_REQ: return "SP_REQ";
        case PO_REQ: return "PO_REQ";
        case NO_EVENT: return "NO_EVENT";
        default: return "UNKNOWN";
    }
}
#endif