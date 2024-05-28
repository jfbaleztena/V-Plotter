#include <stdint.h>
#include <math.h>
#include "draw_tools.h"

// #define BOARD_LENGTH 675
#define BOARD_LENGTH 665
#define MICROSTEP 1

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

void get_step_for_coordinate(step_coordinate* step, int16_t x, int16_t y){
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