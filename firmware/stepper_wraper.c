#include <stdbool.h>
#include "draw_tools.h"
#include "stepper_wraper.h"
#include "stepperCon.h"
#include "vplotter.h"

coordinate_t last_position;

bool movement_done(){
    return sync.step_event_count==0?true:false;
}

bool stepper_wraper_init(){

    stepperSetupIO();
    micros_init(16000000, 40);

    motorx = stepperAdd(200);
    if(motorx == NULL) return false;
    motory = stepperAdd(200);
    if(motory == NULL) return false;
	
    motorx->EN_f = motorX_EN;
    motorx->STEP_f = motorX_STEP;
    motorx->DIR_f = motorX_DIR;
	
    motory->EN_f = motorY_EN;
    motory->STEP_f = motorY_STEP;
    motory->DIR_f = motorY_DIR;

    stepperDefineXYZ(motorx, motory, NULL);
    stepperHoldTorque(motorx, true);
    stepperHoldTorque(motory, true);

    float motor_speed = 100;
    float motor_acceleration = 50;

    stepperSetSpeed(motorx, motor_speed);
    stepperSetAcceleration(motorx, motor_acceleration);

    stepperSetSpeed(motory, motor_speed);
    stepperSetAcceleration(motory, motor_acceleration);

    coordinate_t initial_position;
    get_step_for_coordinate(&initial_position, INITIAL_X, INITIAL_Y);
    stepperSetCurrentPosition(motorx, initial_position.x);
    stepperSetCurrentPosition(motory, initial_position.y);
    
    last_position.x = INITIAL_X;
    last_position.y = INITIAL_Y;

    return true;
}

bool move_absolute(int32_t target_x, int32_t target_y){
    coordinate_t target_position;

    get_step_for_coordinate(&target_position, target_x, target_y);
    int32_t target_arr[1][2] = {{target_position.x, target_position.y}};
    while(1){
        stepperRun();
        if(stepperIsIdleXYZ()){
            stepperMoveToXYZ(target_arr[0]);
        }
        if(movement_done()){
            last_position.x = target_x;
            last_position.y = target_y;
            break;
        } 
    }

    vplot_event_queue(EVT_OK);
    return true;
}

bool move_relative(int32_t delta_x, int32_t delta_y){
    coordinate_t delta_position;
    get_step_for_coordinate(&delta_position, delta_x + last_position.x, delta_y + last_position.y);
    int32_t delta_arr[1][2] = {{delta_position.x, delta_position.y}};

    while(1){
        stepperRun();
        if(stepperIsIdleXYZ()){
            stepperMoveToXYZ(delta_arr[0]);
        }
        if(movement_done()){
            last_position.x += delta_x;
            last_position.y += delta_y;
            break;
        }
    }

    vplot_event_queue(EVT_OK);
    return true;
}