#ifndef __C_SERIAL
#define __C_SERIAL
#include <stdbool.h>

#define RX_BUFFER_SIZE		512
#define RX_LINE_SIZE		128

char  rx_line[RX_LINE_SIZE];
uint16_t  rx_line_pos;
volatile bool frame_ready;

void serial_init();
void serial_char(char data);
void ack();
void nack();
#endif