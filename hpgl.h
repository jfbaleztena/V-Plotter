#ifndef ____HPGL_H__
#define ____HPGL_H__

#include <stdbool.h>
#include <stdint.h>

#define MAX_ARGS 10

typedef enum cmd_t{
	CMD_IN, // Plotter is changed into initial state
	CMD_IP, // (x1,y1;x2,y2) sets the coordiante of the board
	CMD_SC, // (xn1,yn1;xn2,yn2) sets the scale, matching the coordiantes of the board with the new ones
	CMD_SP, // n select the pen to be used, in this case there is only one so support but do not take action
	CMD_PU, // x1,y2(x2,y2..xn.yn) pen up and move to coordiante (or secuence of coordinates)
	CMD_PD, // x1,y2(x2,y2..xn.yn) pen down and move to coordiante (or secuence of coordinates)
	CMD_PA, // x1,y2(x2,y2..xn.yn) pen moved to coordiante (or secuence of coordinates) in an absolute manner	
	CMD_PR, // x1,y2(x2,y2..xn.yn) pen moved to coordiante (or secuence of coordinates) in a relative manner
    CMD_UNKNOWN
}cmd_t;

typedef enum cmd_res_t{
    CMD_OK,
    CMD_ERR_INVALID_CMD,
    CMD_ERR_INVALID_ARG,
    CMD_ERR_INVALID_TERMINATOR,
    CMD_ERR_INVALID_FORMAT
}cmd_res_t;

typedef struct cmd_s{
    cmd_t id;
    int32_t arg[MAX_ARGS];
    uint8_t arg_count;
    cmd_res_t res;
}cmd_s;

void parse_command(char *in_buffer);
void validate_arg();
void clear_cmd();
#endif 