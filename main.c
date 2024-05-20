#include <avr/io.h>
#include <util/delay.h>
#include "custom_serial.h"

int main(){

    serial_init();
    printf("V-PLOTER\r\n");
    
    while(1){
        _delay_ms(1000);
    }
}