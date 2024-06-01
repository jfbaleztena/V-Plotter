#include <stdbool.h>
#include "draw_tools.h"
#include "stepper_wraper.h"
#include "stepperCon.h"
#include "vplotter.h"

coordinate_t last_position;
#define INITIAL_X 350 
#define INITIAL_Y 205

bool movement_done(){
    return sync.step_event_count==0?true:false;
}

bool stepper_wraper_init(){
    // Configure stepper motor pins
    stepperSetupIO();
    // Initiate and start micros timer interrupt (every 40us)
    // used for stepping the motors
    micros_init(16000000, 40);
    // Create a structure object for each driver
    motorx = stepperAdd(200);
    if(motorx == NULL){
        printf("Error creating motorX\r\n");
        return false;
    }else{
        printf("MotorX created OK\r\n");
    }
    motory = stepperAdd(200);
    if(motory == NULL){
        printf("Error creating motorY\r\n");
        return false;
    }else{
        printf("MotorY created OK\r\n");
    } //
	
    // // Assign functions for each driver
    motorx->EN_f = motorX_EN;
    motorx->STEP_f = motorX_STEP;
    motorx->DIR_f = motorX_DIR;
	
    motory->EN_f = motorY_EN;
    motory->STEP_f = motorY_STEP;
    motory->DIR_f = motorY_DIR;

    stepperDefineXYZ(motorx, motory, NULL);
    // Micro stepping factor
    // stepperSetStepDivision(motorx, 16); stepperSetStepDivision(motory, 16);
	
    // Set to true if the coils should remain energized
    // after the movement has been completed
    stepperHoldTorque(motorx, true);
    stepperHoldTorque(motory, true);
    // Set speed and acceleration
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
    int32_t target_arr[1][2] = {target_position.x, target_position.y};
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
    int32_t delta_arr[1][2] = {delta_position.x, delta_position.y};

    printf("Going to x: %d \r\n", delta_arr[0][0]);
    printf("Going to y: %d \r\n", delta_arr[0][1]);

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