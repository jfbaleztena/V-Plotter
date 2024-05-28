#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "custom_serial.h"
#include "hpgl.h"
#include "vplotter.h"

extern vplot_s vplot;

char* get_current_cmd_str();
void print_current_command();
cmd_t get_cmd(char *cmd);

#define MAX_CMD_LENGTH 2 
#define MAX_FRAME_LENGTH 20 
#define SEPARATOR ','
#define TERMINATOR ';'

typedef enum cmd_res_t{
    CMD_OK,
    CMD_ERR_INVALID_CMD,
    CMD_ERR_INVALID_ARG,
    CMD_ERR_INVALID_TERMINATOR,
    CMD_ERR_INVALID_FORMAT
}cmd_res_t;

cmd_t get_cmd(char *cmd){
    if(strcmp(cmd, "IN")==0) return CMD_IN;
    if(strcmp(cmd, "IP")==0) return CMD_IP;
    if(strcmp(cmd, "SC")==0) return CMD_SC;
    if(strcmp(cmd, "SP")==0) return CMD_SP;
    if(strcmp(cmd, "PU")==0) return CMD_PU;
    if(strcmp(cmd, "PD")==0) return CMD_PD;
    if(strcmp(cmd, "PA")==0) return CMD_PA;
    if(strcmp(cmd, "PR")==0) return CMD_PR;
    return CMD_UNKNOWN;
}

void print_current_command(){
    printf("Command: %s\r\n", get_current_cmd_str());
    printf("Args: ");
    for(uint8_t i=0; i < vplot.cmd.arg_count; i++){
        printf("%d ", vplot.cmd.arg[i]);
    }
    printf("\r\n");
}

char* get_current_cmd_str(){
    char* cmd_str;
    switch(vplot.cmd.id){
        case CMD_IN:
            cmd_str = "CMD_IN";
            break;
        case CMD_IP:
            cmd_str = "CMD_IP";
            break;
        case CMD_SC:
            cmd_str = "CMD_SC";
            break;
        case CMD_SP:
            cmd_str = "CMD_SP";
            break;
        case CMD_PU:
            cmd_str = "CMD_PU";
            break;
        case CMD_PD:
            cmd_str = "CMD_PD";
            break;
        case CMD_PA:
            cmd_str = "CMD_PA";
            break;
        case CMD_PR:
            cmd_str = "CMD_PR";
            break;
        case CMD_UNKNOWN:
            cmd_str = "CMD_UNKNOWN";
            break;
    }
    return cmd_str;
}

void handle_res(){
    switch(vplot.cmd.res){
        case CMD_OK:
            printf("Command parsed correctly\r\n");
            break;
        case CMD_ERR_INVALID_CMD:
            printf("Invalid command\r\n");
            break;
        case CMD_ERR_INVALID_ARG:
            printf("Invalid argument\r\n");
            break;
        case CMD_ERR_INVALID_TERMINATOR:
            printf("Invalid terminator\r\n");
            break;
        case CMD_ERR_INVALID_FORMAT:
            printf("Invalid format\r\n");
            break;
    }
}

void parse_command(char *in_buffer){
    printf("Parsing command: %s\r\n", in_buffer);
    char cmd[3];
    char arg[10];

    if(!isdigit(in_buffer[MAX_CMD_LENGTH]) && in_buffer[MAX_CMD_LENGTH]!=TERMINATOR){
        vplot.cmd.res = CMD_ERR_INVALID_FORMAT;
        goto parse_end;
    }
    
    cmd[0] = in_buffer[0]; cmd[1] = in_buffer[1]; cmd[2] = '\0';
    vplot.cmd.id = get_cmd(cmd);
    if(vplot.cmd.id == CMD_UNKNOWN){
        vplot.cmd.res = CMD_ERR_INVALID_CMD;
        goto parse_end;
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
            goto parse_end;
        }
    } 

    if(in_buffer[strlen(in_buffer)-1] != TERMINATOR){
        vplot.cmd.res = CMD_ERR_INVALID_TERMINATOR;
        goto parse_end;
    }

    vplot.cmd.res = CMD_OK;
    vplot_fsm(VALID_STRING);
    return;

parse_end:
    handle_res();
    vplot_fsm(INVALID_STRING);
    return;
}