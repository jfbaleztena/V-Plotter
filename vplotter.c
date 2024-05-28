#include "vplotter.h"
#include "custom_serial.h"
#include "hpgl.h"

vplot_s vplot;

void vplot_init(){
    vplot.state = IDLE;
    vplot.event = NO_EVENT;
}

void vplot_fsm(event_t new_event){
    switch(vplot.state){
        case IDLE:
            if(new_event == NEW_STRING){
                printf("FSM: IDLE -> HPGL_PARSE\r\n");
                vplot.state = HPGL_PARSE;
                parse_command(rx_line);
            }
            break;

        case HPGL_PARSE:
            switch(new_event){
                case INVALID_STRING:
                    printf("FSM: HPGL_PARSE -> IDLE\r\n");
                    vplot.state = IDLE;
                    break;
                case PA_REQ:
                    printf("FSM: HPGL_PARSE -> PA\r\n");
                    vplot.state = PA;
                    break;
                case PU_REQ:
                    printf("FSM: HPGL_PARSE -> PU\r\n");
                    vplot.state = PU;
                    break;
                case PD_REQ:
                    printf("FSM: HPGL_PARSE -> PD\r\n");
                    vplot.state = PD;
                    break;
                case PR_REQ:
                    printf("FSM: HPGL_PARSE -> PR\r\n");
                    vplot.state = PR;
                    break;
                case IP_REQ:
                    printf("FSM: HPGL_PARSE -> BOARD_CONF(IP)\r\n");
                    vplot.state = BOARD_CONF;
                    break;
                case IN_REQ:
                    printf("FSM: HPGL_PARSE -> INITIALIZE\r\n");
                    vplot.state = INITIALIZE;
                    break;
                case SC_REQ:
                    printf("FSM: HPGL_PARSE -> BOARD_CONF(SC)\r\n");
                    vplot.state = BOARD_CONF;
                    break;
                case SP_REQ:
                    printf("FSM: HPGL_PARSE -> IDLE(SP)\r\n");
                    vplot.state = IDLE;
                    ack();
                    break;
                default: break;
            }
            break;
        case INITIALIZE:
            if(new_event == OK){
                printf("FSM: INITIALIZE -> IDLE\r\n");
                ack();
            }else if(new_event == FAIL){
                printf("FSM: INITIALIZE -> IDLE\r\n");
                nack();
            }
            vplot.state = IDLE;
            break;
        case PA:
            if(new_event == OK){
                printf("FSM: PA -> IDLE\r\n");
                vplot.state = IDLE;
                ack();
            }else if(new_event == FAIL){
                printf("FSM: PA -> EM_STOP\r\n");
                vplot.state = EM_STOP;
            }else if(new_event == INVALID_ARG){
                printf("FSM: PA -> IDLE\r\n");
                vplot.state = IDLE;
                nack();
            }
            break;

        case PU:
            if(new_event == OK){
                printf("FSM: PU -> IDLE\r\n");
                vplot.state = IDLE;
                ack();
            }else if(new_event == FAIL){
                printf("FSM: PU -> EM_STOP\r\n");
                vplot.state = EM_STOP;
            }else if(new_event == INVALID_ARG){
                printf("FSM: PU -> IDLE\r\n");
                vplot.state = IDLE;
                nack();
            }
            break;

        case PD:
            if(new_event == OK){
                printf("FSM: PD -> IDLE\r\n");
                vplot.state = IDLE;
                ack();
            }else if(new_event == FAIL){
                printf("FSM: PD -> EM_STOP\r\n");
                vplot.state = EM_STOP;
            }else if(new_event == INVALID_ARG){
                printf("FSM: PD -> IDLE\r\n");
                vplot.state = IDLE;
                nack();
            }
            break;

        case PR:
            if(new_event == OK){
                printf("FSM: PR -> IDLE\r\n");
                vplot.state = IDLE;
                ack();
            }else if(new_event == FAIL){
                printf("FSM: PR -> EM_STOP\r\n");
                vplot.state = EM_STOP;
            }else if(new_event == INVALID_ARG){
                printf("FSM: PR -> IDLE\r\n");
                vplot.state = IDLE;
                nack();
            }
            break;

        case BOARD_CONF:
            if(new_event == OK){
                printf("FSM: BOARD_CONF -> IDLE\r\n");
                vplot.state = IDLE;
                ack();
            }else if(new_event == FAIL){
                printf("FSM: BOARD_CONF -> EM_STOP\r\n");
                vplot.state = EM_STOP;
            }else if(new_event == INVALID_ARG){
                printf("FSM: BOARD_CONF -> IDLE\r\n");
                vplot.state = IDLE;
                nack();
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