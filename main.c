#include <avr/io.h>
#include <util/delay.h>
#include "custom_serial.h"
#include "stepperCon.h"

#ifndef F_CPU
    #warning	"F_CPU not defined. Define it in project properties."
#elif F_CPU != 16000000
    #warning	"Wrong F_CPU frequency!"
#endif

int main(void){
	
    serial_init();
    printf("V-PLOTER\r\n");
    // Configure stepper motor pins
    stepperSetupIO();
	
    // Initiate and start micros timer interrupt (every 40us)
    // used for stepping the motors
    micros_init(16000000, 40);
	
    // Create a structure object for each driver
    stepperCon* motorx = stepperAdd(200);
    if(motorx == NULL){
        printf("Error creating motorX\r\n");
    }else{
        printf("MotorX created OK\r\n");
    }
    stepperCon* motory = stepperAdd(200);
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

    // Micro stepping factor
    stepperSetStepDivision(motorx, 1);
    stepperSetStepDivision(motory, 1);
	
    // Set to true if the coils should remain energized
    // after the movement has been completed
    stepperHoldTorque(motorx, false);
    stepperHoldTorque(motory, false);

    // Set speed and acceleration
    stepperSetSpeed(motorx, 2000);
    stepperSetAcceleration(motorx, 150);
	
    stepperSetSpeed(motory, 2000);
    stepperSetAcceleration(motory, 150);
    // Move n steps
    // stepperMove(motorx, 10000);
    uint8_t direction_flag = 0;
    // while(1);
    while(1){
// This only moves the motor(s) after a move function is used. 
// Used to create segments executed by the ISR.
        stepperRun();
// Check when movement is complete
// and execute another one
        if(stepperIsIdle(motorx) && stepperIsIdle(motory)){
            _delay_ms(2000);
            if(direction_flag == 0){
                // stepperToDegree(motory, 90);
                stepperMove(motorx, 200);
                stepperMove(motory, 200);
                direction_flag = 1;
            }else{
                direction_flag = 0;
                stepperMove(motorx, -200); // move in the opposite direction
                stepperMove(motory, -200); // move in the opposite direction
            }
            
        }
    }
}