#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "draw_tools.h"
#include "vplotter.h"
#include "custom_serial.h"

// #define BOARD_LENGTH 675
#define BOARD_LENGTH 665
#define MICROSTEP 1

board_s board;

void get_board_coordinates(){
    printf("Board coordinates: x{%d,%d},y{%d,%d}\r\n", board.x1, board.x2, board.y1, board.y2);
}

void set_board_coodinates(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    if(x1 > BOARD_LENGTH || x2 > BOARD_LENGTH || y1 > BOARD_LENGTH || y2 > BOARD_LENGTH){
       vplot_event_queue(EVT_FAIL);
       return;
    }

    board.x1 = x1;
    board.x2 = x2;
    board.y1 = y1;
    board.y2 = y2;

    vplot_event_queue(EVT_OK);
}

int32_t get_step_for_lenght(float length){
    float steps = 2000*length/383; 
    return (int32_t)(MICROSTEP*steps);
}

float get_initial_x_length(uint16_t x_mm, uint16_t y_mm){
    return sqrt(pow(x_mm,2) + pow(y_mm,2));
}

float get_initial_y_length(uint16_t x_mm, uint16_t y_mm){
    return sqrt(pow(BOARD_LENGTH - x_mm,2) + pow(y_mm,2));
}

void get_step_for_coordinate(coordinate_t* step, int16_t x, int16_t y){
    printf("---\r\nReq coordinate: %d, %d\r\n", x, y);
    float x_length = get_initial_x_length(x, y);
    float y_length = get_initial_y_length(x, y);
    step->x = get_step_for_lenght(x_length);
    step->y = get_step_for_lenght(y_length);
    printf("Length x-belt: %d", (uint32_t)x_length);
    printf(" / %d steps\r\n",  step->x);
    printf("Length y-belt: %d", (uint32_t)y_length);
    printf(" / %d steps\r\n",  step->y);
    printf("---\r\n");
}