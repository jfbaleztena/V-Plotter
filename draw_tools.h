#ifndef ___DRAW_TOOLS_H___
#define ___DRAW_TOOLS_H___

#include <stdint.h>

#define OFFSET_X 222
#define OFFSET_Y 260

typedef struct step_coordinate{
    int32_t x;
    int32_t y;
}step_coordinate;

int32_t get_step_for_lenght(float length);
float get_initial_x_length(uint16_t x_mm, uint16_t y_mm);
float get_initial_y_length(uint16_t x_mm, uint16_t y_mm);
void get_step_for_coordinate(step_coordinate* step, int16_t x, int16_t y);
#endif