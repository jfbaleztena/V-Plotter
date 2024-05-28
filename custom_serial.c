#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "custom_serial.h"
#include "hpgl.h"

static const int USART_BAUDRATE  = 9600;
static inline int UBRR_VALUE(){return(((F_CPU/(USART_BAUDRATE*16UL)))-1);}

static bool rx_first_byte = 0;
static char rx_buffer[RX_BUFFER_SIZE]; // debug buffer


ISR(USART_RX_vect){
	if(rx_first_byte == 0) memset(rx_line, 0, RX_LINE_SIZE); // wipe rx buffer before loading

	rx_first_byte = 1; // Block

	unsigned char input = UDR0; 	// Receive data

	if(input != '\n')	rx_line[rx_line_pos++] = input; // save bytes until 'end of line'
	if(rx_line_pos >= RX_LINE_SIZE || (input == '\n' && rx_line_pos > 0)) {
		// if MAX size, or end of line then process
		rx_line_pos = 0;
		rx_first_byte = 0;

		frame_ready = true;
	}
}
	
FILE uart_str = FDEV_SETUP_STREAM(serial_char, NULL, _FDEV_SETUP_RW);

void serial_init(){
	UBRR0 = UBRR_VALUE();			// baud_rate
	UCSR0B|=(1<<TXEN0);			// TXen
	UCSR0B|=(1<<RXEN0);			// RXen
	UCSR0B|=(1<<RXCIE0);		// RX int
	UCSR0C = 0x06;			 	// 8N1
	
	sei();				// global int en

    stdout=&uart_str;
	frame_ready = false;
	printf("Serial Initialized\r\n");
}

void serial_char(char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void ack(){
	printf("ACK\r\n");
	clear_cmd();
}

void nack(){
	printf("NACK\r\n");
	clear_cmd();
}