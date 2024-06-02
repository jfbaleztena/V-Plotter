#ifndef ___DRAW_TOOLS_H___
#define ___DRAW_TOOLS_H___

#include <stdint.h>

#define OFFSET_X 222
#define OFFSET_Y 260

#define INITIAL_X 350 
#define INITIAL_Y 205

#define BOARD_LENGTH 665

typedef struct coordinate_t{
    int32_t x;
    int32_t y;
}coordinate_t;

typedef struct board_s{
    uint16_t x1,x2;
    uint16_t y1,y2;
}board_s;

void get_board_coordinates();
void set_board_coodinates(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
int32_t get_step_for_lenght(float length);
float get_initial_x_length(uint16_t x_mm, uint16_t y_mm);
float get_initial_y_length(uint16_t x_mm, uint16_t y_mm);
void get_step_for_coordinate(coordinate_t* step, int16_t x, int16_t y);
#endif