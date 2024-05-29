#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "vplotter.h"
#include "custom_serial.h"
#include "hpgl.h"
#include "stepper_wraper.h"
#include "draw_tools.h"
#include "queue.h"

volatile vplot_s vplot;

void vplot_init(){
    vplot.state = IDLE;
    vplot.event = NO_EVENT;
    event_queue.head = NULL;
    event_queue.tail = NULL;
}

void vplot_mechanical_init(){
    if(servo_init() == true){
        if(stepper_wraper_init() == true){
            vplot_fsm(OK);
            return;
        }
    }
    vplot_fsm(FAIL);
}

void vplot_fsm(event_t new_event){
    switch(vplot.state){
        case IDLE:
            if(new_event == NEW_STRING){
                vplot.state = HPGL_PARSE;
                printf("FSM: IDLE -> HPGL_PARSE\r\n");
                parse_command(rx_line);
            }else{
                printf("FSM: IDLE ERR, UNK EVENT\r\n");
            }
            break;

        case HPGL_PARSE:
            switch(new_event){
                case INVALID_STRING:
                    vplot.state = IDLE;
                    printf("FSM: HPGL_PARSE -> IDLE\r\n");
                    nack();
                    break;
                case PA_REQ:
                    vplot.state = PA;
                    printf("FSM: HPGL_PARSE -> PA\r\n");
                    break;
                case PU_REQ:
                    vplot.state = PU;
                    printf("FSM: HPGL_PARSE -> PU\r\n");
                    break;
                case PD_REQ:
                    vplot.state = PD;
                    validate_arg();
                    printf("FSM: HPGL_PARSE -> PD\r\n");
                    break;
                case PR_REQ:
                    vplot.state = PR;
                    printf("FSM: HPGL_PARSE -> PR\r\n");
                    break;
                case IP_REQ:
                    vplot.state = BOARD_CONF;
                    printf("FSM: HPGL_PARSE -> BOARD_CONF(IP)\r\n");
                    validate_arg();
                    break;
                case IN_REQ:
                    vplot.state = INITIALIZE;
                    printf("FSM: HPGL_PARSE -> INITIALIZE\r\n");
                    vplot_mechanical_init();
                    break;
                case SC_REQ:
                    vplot.state = BOARD_CONF;
                    printf("FSM: HPGL_PARSE -> BOARD_CONF(SC)\r\n");
                    break;
                case SP_REQ:
                    vplot.state = IDLE;
                    printf("FSM: HPGL_PARSE -> IDLE(SP)\r\n");
                    ack();
                    break;
                default: break;
            }
            break;

        case INITIALIZE:
            vplot.state = IDLE;
            if(new_event == OK){
                printf("FSM: INITIALIZE -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL){
                printf("FSM: INITIALIZE -> IDLE\r\n");
                nack();
            }
            break;

        case PA:
            if(new_event == OK){
                vplot.state = IDLE;
                printf("FSM: PA -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL){
                vplot.state = EM_STOP;
                printf("FSM: PA -> EM_STOP\r\n");
            }else if(new_event == INVALID_ARG){
                vplot.state = IDLE;
                printf("FSM: PA -> IDLE\r\n");
                nack();
            }
            break;

        case PU:
            if(new_event == OK){
                vplot.state = IDLE;
                printf("FSM: PU -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL){
                vplot.state = EM_STOP;
                printf("FSM: PU -> EM_STOP\r\n");
            }else if(new_event == INVALID_ARG){
                vplot.state = IDLE;
                printf("FSM: PU -> IDLE\r\n");
                nack();
            }
            break;

        case PD:
            if(new_event == OK){
                vplot.state = IDLE;
                printf("FSM: PD -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL || new_event == INVALID_ARG){
                vplot.state = IDLE;
                printf("FSM: PD -> IDLE\r\n");
                nack();
            }else if(new_event == VALID_ARG){
                vplot.state = PD;
                printf("FSM: PD -> PD\r\n");
                pen_down();
                // get_step_for_coordinate(&vplot.cmd.step, vplot.cmd.arg[0], vplot.cmd.arg[1]);
                // move_to(vplot.cmd.step.x, vplot.cmd.step.y);
            }else{
                printf("FSM: PD ERR, UNK EVENT %d\r\n", new_event);
            }
            break;

        case PR:
            if(new_event == OK){
                vplot.state = IDLE;
                printf("FSM: PR -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL){
                vplot.state = EM_STOP;
                printf("FSM: PR -> EM_STOP\r\n");
            }else if(new_event == INVALID_ARG){
                vplot.state = IDLE;
                printf("FSM: PR -> IDLE\r\n");
                nack();
            }
            break;

        case BOARD_CONF:
            if(new_event == OK){
                vplot.state = IDLE;
                printf("FSM: BOARD_CONF -> IDLE\r\n");
                get_board_coordinates();
                ack();
            }else if(new_event == FAIL || new_event == INVALID_ARG){
                vplot.state = IDLE;
                printf("FSM: BOARD_CONF -> IDLE\r\n");
                nack();
            }else if(new_event == VALID_ARG){
                vplot.state = BOARD_CONF;
                printf("FSM: BOARD_CONF -> BOARD_CONF\r\n");
                set_board_coodinates(vplot.cmd.arg[0], vplot.cmd.arg[1], vplot.cmd.arg[2], vplot.cmd.arg[3]);
            }
            break;
            
        case EM_STOP:
            printf("FSM: EM_STOP -> IDLE\r\n");
            vplot.state = IDLE;
            nack();
            break;

        default: break; 
    }
}