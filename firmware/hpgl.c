#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "custom_serial.h"
#include "hpgl.h"
#include "vplotter.h"

extern vplot_s vplot;

char* get_current_cmd_str();
cmd_t get_cmd(char *cmd);
void validate_arg();

#define MAX_CMD_LENGTH 2 
#define MAX_FRAME_LENGTH 100 
#define SEPARATOR ','
#define TERMINATOR ';'

void clear_cmd(){
    vplot.cmd.id = CMD_UNKNOWN;
    vplot.cmd.arg_count = 0;
    vplot.cmd.arg_used = 0;
    vplot.cmd.res = CMD_UNKNOWN;
}

event_t cmd_to_event(){
    switch(vplot.cmd.id){
        case CMD_PU:
            return PU_REQ;
        case CMD_PD:
            return PD_REQ;
        case CMD_PA:
            return PA_REQ;
        case CMD_PR:
            return PR_REQ;
        case CMD_IP:
            return IP_REQ;
        case CMD_IN:
            return IN_REQ;
        case CMD_SC:
            return SC_REQ;
        case CMD_SP:
            return SP_REQ;
        case CMD_PO:
            return PO_REQ;
        default:
            return INVALID_STRING;
    }
}

cmd_t get_cmd(char *cmd){
    if(strcmp(cmd, "IN")==0) return CMD_IN;
    if(strcmp(cmd, "IP")==0) return CMD_IP;
    if(strcmp(cmd, "SC")==0) return CMD_SC;
    if(strcmp(cmd, "SP")==0) return CMD_SP;
    if(strcmp(cmd, "PU")==0) return CMD_PU;
    if(strcmp(cmd, "PD")==0) return CMD_PD;
    if(strcmp(cmd, "PA")==0) return CMD_PA;
    if(strcmp(cmd, "PR")==0) return CMD_PR;
    if(strcmp(cmd, "PO")==0) return CMD_PO;
    return CMD_UNKNOWN;
}

void handle_err(){
    switch(vplot.cmd.res){
        case CMD_OK:
            break;
        case CMD_ERR_INVALID_CMD:
            printf("E: CMD\r\n");
            break;
        case CMD_ERR_INVALID_ARG:
            printf("E: ARG\r\n");
            break;
        case CMD_ERR_INVALID_TERMINATOR:
            printf("E: TERM\r\n");
            break;
        case CMD_ERR_INVALID_FORMAT:
            printf("E: FORMAT\r\n");
            break;
    }
}

void parse_command(char *in_buffer){
    printf("REQ-CMD: %s\r\n", in_buffer);
    char cmd[3];
    char arg[10];

    if(!isdigit(in_buffer[MAX_CMD_LENGTH]) && in_buffer[MAX_CMD_LENGTH]!=TERMINATOR && in_buffer[MAX_CMD_LENGTH]!='-'){
        vplot.cmd.res = CMD_ERR_INVALID_FORMAT;
        goto parse_fail;
    }
    
    cmd[0] = in_buffer[0]; cmd[1] = in_buffer[1]; cmd[2] = '\0';
    vplot.cmd.id = get_cmd(cmd);
    if(vplot.cmd.id == CMD_UNKNOWN){
        vplot.cmd.res = CMD_ERR_INVALID_CMD;
        goto parse_fail;
    }

    if(in_buffer[MAX_CMD_LENGTH] == TERMINATOR){
        goto parse_ok;
    }

    uint8_t arg_index = 0;

    for(uint8_t i=0; i<=strlen(in_buffer); i++){
        if(in_buffer[MAX_CMD_LENGTH+i] == SEPARATOR || in_buffer[MAX_CMD_LENGTH+i] == TERMINATOR){
            arg[arg_index] = '\0';
            arg_index = 0;
            vplot.cmd.arg[vplot.cmd.arg_count] = atoi(arg);
            vplot.cmd.arg_count++;
        }else{
            arg[arg_index] = in_buffer[MAX_CMD_LENGTH+i];
            arg_index++;
        }
        if (i> MAX_FRAME_LENGTH){
            vplot.cmd.res = CMD_ERR_INVALID_FORMAT;
            goto parse_fail;
        }
    } 

    if(in_buffer[strlen(in_buffer)-1] != TERMINATOR){
        vplot.cmd.res = CMD_ERR_INVALID_TERMINATOR;
        goto parse_fail;
    }

parse_ok:
    vplot.cmd.res = CMD_OK;
    vplot_event_queue(cmd_to_event());
    return;

parse_fail:
    handle_err();
    vplot_event_queue(INVALID_STRING);
    return;
}

void validate_arg(){
    switch(vplot.cmd.id){
        case(CMD_PA):
            goto validate_arg_ok;

        case(CMD_IP):
            if(vplot.cmd.arg_count != 4){
                // printf("Invalid argument for IP command: amount of args error\r\n");
                goto validate_arg_fail;
            }

            if(vplot.cmd.arg[0] < 0 || vplot.cmd.arg[1] < 0 || vplot.cmd.arg[2] < 0 || vplot.cmd.arg[3] < 0){
                // printf("Invalid argument for IP command: signess error\r\n");
                goto validate_arg_fail;
            }
            if(vplot.cmd.arg[0] < vplot.cmd.arg[2] || vplot.cmd.arg[1] < vplot.cmd.arg[3]){
                // printf("Invalid argument for IP command: x1,y1 > x2,y2 error\r\n");
                goto validate_arg_fail;
            }
            goto validate_arg_ok;
            break;
        case(CMD_PD):
            if(vplot.cmd.arg_count == 0){
                goto validate_arg_ok;
            }else if(vplot.cmd.arg_count % 2 != 0){
                // printf("Invalid argument for PD command: odd number of args \r\n");
                goto validate_arg_fail;
            }
            for(uint8_t i=0; i<vplot.cmd.arg_count; i++){
                if(vplot.cmd.arg[i] < 0){
                    // printf("Invalid argument for PD command: signess error\r\n");
                    goto validate_arg_fail;
                }
            }
            goto validate_arg_ok;
            break;

        default:break;
        // default: printf("Invalid command for argument validation\r\n"); goto validate_arg_fail; break;
    }

validate_arg_ok:
    vplot_event_queue(VALID_ARG);
    return;

validate_arg_fail:
    vplot_event_queue(INVALID_ARG);
    return;
}