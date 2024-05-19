#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "custom_serial.h"

static const int USART_BAUDRATE  = 9600;
static inline int UBRR_VALUE(){return(((F_CPU/(USART_BAUDRATE*16UL)))-1);}
	
static bool rx_first_byte = 0;
#ifdef PRINT_SYSTEM_INFO
static char rx_buffer[RX_BUFFER_SIZE]; // debug buffer
#endif

// ISR(USART_RX_vect)
// {
// 	if(rx_first_byte == 0) memset(rx_line, 0, RX_LINE_SIZE); // wipe rx buffer before loading

// 	rx_first_byte = 1; // Block

// 	unsigned char input = UDR0; 	// Receive data
// 	if(input != '\n')	rx_line[rx_line_pos++] = input; // save bytes until 'end of line'

// 	if(rx_line_pos >= RX_LINE_SIZE || (input == '\n' && rx_line_pos > 0)) {
// 		// if MAX size, or end of line then process
// 		rx_line_pos = 0;
// 		rx_first_byte = 0;
// 		state_machine.prev_state = state_machine.state;
// 		state_machine.state = sm_parser();
// 	}
// }

void serial_init(){
	UBRR0 = UBRR_VALUE();			// baud_rate
	UCSR0B|=(1<<TXEN0);			// TXen
	UCSR0B|=(1<<RXEN0);			// RXen
	UCSR0B|=(1<<RXCIE0);		// RX int
	UCSR0C = 0x06;			 	// 8N1
	
	sei();				// global int en
	
	rx_line_pos = 0;
	rx_first_byte = 0;
}

void serial_char(char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void serial_string(const char* s) {
	while (*s != '\0')
	serial_char(*s++);
}