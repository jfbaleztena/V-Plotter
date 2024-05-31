#ifndef STEPPER_WRAPER_H
#define STEPPER_WRAPER_H

#include <stdbool.h>
#include "stepperCon.h"

stepperCon* motorx;
stepperCon* motory;

bool stepper_wraper_init();
bool move_absolute(int32_t target_x, int32_t target_y);
#endif