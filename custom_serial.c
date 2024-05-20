#include <avr/io.h>
#include <stdio.h>
#include "custom_serial.h"

static const int USART_BAUDRATE  = 9600;
static inline int UBRR_VALUE(){return(((F_CPU/(USART_BAUDRATE*16UL)))-1);}
	
FILE uart_str = FDEV_SETUP_STREAM(serial_char, NULL, _FDEV_SETUP_RW);

void serial_init(){
	UBRR0 = UBRR_VALUE();			// baud_rate
	UCSR0B|=(1<<TXEN0);			// TXen
	UCSR0B|=(1<<RXEN0);			// RXen
	UCSR0B|=(1<<RXCIE0);		// RX int
	UCSR0C = 0x06;			 	// 8N1
	
    stdout=&uart_str;
	printf("Serial Initialized\r\n");
}

void serial_char(char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}