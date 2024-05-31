#include <avr/io.h>
#include <util/delay.h>
#include "custom_serial.h"
#include "vplotter.h"

#ifndef F_CPU
    #warning	"F_CPU not defined. Define it in project properties."
#elif F_CPU != 16000000
    #warning	"Wrong F_CPU frequency!"
#endif

extern vplot_s vplot;

int main(void){
	
    serial_init();
    printf("V-PLOTER\r\n");
    vplot_init();

    while(1){
        vplot_run();
    }

}


    // #define MAX_POINTS 16 
    // uint16_t coordinates[MAX_POINTS][2] = {
    //     {0, 100},
    //     {0, 200},
    //     {100, 400},
    //     {200, 400},
    //     {400, 300},
    //     {400, 200},
    //     {400, 100},
    //     {400, 0},
    //     {300, 0},
    //     {200, 0},
    //     {100, 0},
    //     {0, 0}
    // };
    
    // step_coordinate instructions[MAX_POINTS];

    // get_step_for_coordinate(&instructions[MAX_POINTS-1], coordinates[MAX_POINTS-1][0]+OFFSET_X, coordinates[MAX_POINTS-1][1]+OFFSET_Y);
    // stepperSetCurrentPosition(motorx, instructions[MAX_POINTS-1].x);
    // stepperSetCurrentPosition(motory, instructions[MAX_POINTS-1].y);

    // int32_t xy[MAX_POINTS][2];
    // for(uint8_t i=0; i<MAX_POINTS; i++){
    //     get_step_for_coordinate(&instructions[i], coordinates[i][0]+OFFSET_X, coordinates[i][1]+OFFSET_Y);
    //     xy[i][0] = instructions[i].x;
    //     xy[i][1] = instructions[i].y;
    //     // printf("instruction %d is %d, %d\r\n", i, instructions[i].x, instructions[i].y);
    //     // printf("xy %d is %d, %d\r\n", i, xy[i][0], xy[i][1]);
    // }
	
    // // while(1);
    // // _delay_ms(2000);
    // uint8_t j = 0;
    // bool draw = true;
    // while(1){

    //     stepperRun();
		
    //     if(draw && stepperIsIdleXYZ()){
    //         // _delay_ms(20);
    //         printf("[%d] Moving to %d(%d),%d(%d)\r\n", j, coordinates[j][0], xy[j][0], coordinates[j][1], xy[j][1]);
    //         stepperMoveToXYZ(xy[j]);
    //         j++;
    //         if(j == MAX_POINTS) draw = false;
    //     }
    // }