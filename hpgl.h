#ifndef ____HPGL_H__
#define ____HPGL_H__
#include <stdbool.h>

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

typedef enum cmd_res_t cmd_res_t;
typedef struct cmd_s;

cmd_res_t parse_command(char *in_buffer);
#endif 