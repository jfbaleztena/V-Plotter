#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "custom_serial.h"
#include "stepperCon.h"
#include "math.h"
#include "servo.h"
#include "vplotter.h"

#ifndef F_CPU
    #warning	"F_CPU not defined. Define it in project properties."
#elif F_CPU != 16000000
    #warning	"Wrong F_CPU frequency!"
#endif

// #define BOARD_LENGTH 675
#define BOARD_LENGTH 665
#define MICROSTEP 1

stepperCon* motorx;
stepperCon* motory;

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

typedef struct step_coordinate{
    int32_t x;
    int32_t y;
}step_coordinate;

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

int main(void){
	
    serial_init();
    servo_init();

    printf("V-PLOTER\r\n");

    pencil_lift();
    while(1);
    
    // Configure stepper motor pins
    stepperSetupIO();
	
    // Initiate and start micros timer interrupt (every 40us)
    // used for stepping the motors
    micros_init(16000000, 40);
	
    // Create a structure object for each driver
    motorx = stepperAdd(200);
    if(motorx == NULL){
        printf("Error creating motorX\r\n");
    }else{
        printf("MotorX created OK\r\n");
    }
    motory = stepperAdd(200);
    if(motory == NULL){
        printf("Error creating motorY\r\n");
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

    #define OFFSET_X 222
    #define OFFSET_Y 260


    #define MAX_POINTS 16 
    uint16_t coordinates[MAX_POINTS][2] = {
        {0, 100},
        {0, 200},
        {100, 400},
        {200, 400},
        {400, 300},
        {400, 200},
        {400, 100},
        {400, 0},
        {300, 0},
        {200, 0},
        {100, 0},
        {0, 0}
    };
    
    step_coordinate instructions[MAX_POINTS];

    get_step_for_coordinate(&instructions[MAX_POINTS-1], coordinates[MAX_POINTS-1][0]+OFFSET_X, coordinates[MAX_POINTS-1][1]+OFFSET_Y);
    stepperSetCurrentPosition(motorx, instructions[MAX_POINTS-1].x);
    stepperSetCurrentPosition(motory, instructions[MAX_POINTS-1].y);

    int32_t xy[MAX_POINTS][2];
    for(uint8_t i=0; i<MAX_POINTS; i++){
        get_step_for_coordinate(&instructions[i], coordinates[i][0]+OFFSET_X, coordinates[i][1]+OFFSET_Y);
        xy[i][0] = instructions[i].x;
        xy[i][1] = instructions[i].y;
        // printf("instruction %d is %d, %d\r\n", i, instructions[i].x, instructions[i].y);
        // printf("xy %d is %d, %d\r\n", i, xy[i][0], xy[i][1]);
    }
	
    // while(1);
    // _delay_ms(2000);
    uint8_t j = 0;
    bool draw = true;
    while(1){

        stepperRun();
		
        if(draw && stepperIsIdleXYZ()){
            // _delay_ms(20);
            printf("[%d] Moving to %d(%d),%d(%d)\r\n", j, coordinates[j][0], xy[j][0], coordinates[j][1], xy[j][1]);
            stepperMoveToXYZ(xy[j]);
            j++;
            if(j == MAX_POINTS) draw = false;
        }
    }
    while(1){
        if(frame_ready == true){
            printf("Frame ready\r\n");
            frame_ready = false;
            vplot_fsm(NEW_STRING);
        }
}