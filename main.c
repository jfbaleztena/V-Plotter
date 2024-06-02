#include <avr/io.h>
#include "custom_serial.h"
#include "vplotter.h"


extern vplot_s vplot;

int main(void){
	
    serial_init();
    printf("V-PLOTER\r\n");
    vplot_init();

    while(1){
        vplot_run();
    }

}