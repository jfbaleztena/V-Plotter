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
	
    // Assign functions for each driver
    motorx->EN_f = motorX_EN;
    motorx->STEP_f = motorX_STEP;
    motorx->DIR_f = motorX_DIR;
	
    // Micro stepping factor
    stepperSetStepDivision(motorx, 1);
	
    // Set to true if the coils should remain energized
    // after the movement has been completed
    stepperHoldTorque(motorx, false);

    // Set speed and acceleration
    stepperSetSpeed(motorx, 2000);
    stepperSetAcceleration(motorx, 150);
	
    // Move n steps
    // stepperMove(motorx, 10000);
    uint8_t direction_flag = 0;
    while(1){
// This only moves the motor(s) after a move function is used. 
// Used to create segments executed by the ISR.
        stepperRun();
// Check when movement is complete
// and execute another one
        if(stepperIsIdle(motorx)){
            _delay_ms(2000);
            // switch(direction_flag){
            //     case 0:
            //         printf("Moving to 90 degrees\r\n");
            //         stepperToDegree(motorx, 90);
            //         direction_flag++;
            //         break;
            //     case 1:
            //     printf("Moving to 180 degrees\r\n");
            //         stepperToDegree(motorx, 180);
            //         direction_flag++;
            //         break;
            //     case 2:
            //     printf("Moving to 270 degrees\r\n");
            //         stepperToDegree(motorx, 270);
            //         direction_flag++;
            //         break;
            //     case 3:
            //     printf("Moving to 360 degrees\r\n");
            //         stepperToDegree(motorx, 360);
            //         direction_flag = 0;
            //         break;
            //     default: break;
            // }
            if(direction_flag == 0){
                stepperToDegree(motorx, 90);
                direction_flag = 1;
                stepperMove(motorx, 200); // move in one direction
            }else{
                direction_flag = 0;
                stepperMove(motorx, -200); // move in the opposite direction
            }
            
        }
    }
}