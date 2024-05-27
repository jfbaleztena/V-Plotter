#ifndef __SERVO_H_
#define __SERVO_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define USE_CHANNELS 2

#define PRESCALER 8UL
#define F_PWM 50UL

#define CHANNEL_A_PIN PB3
#define CHANNEL_A_PORT PORTB
#define CHANNEL_A_DDR DDRB

#define F_CALLIBRATION 60


#define MULTIPLAYER (F_CPU / 1000000UL) / PRESCALER
#define FREQUENCY_OCR F_CPU / PRESCALER / F_PWM - 1

#define CHANNEL_A 0
#define CHANNEL_A_REG OCR1A

#define CHANNEL_A_OUT CHANNEL_A_DDR |= (1 << CHANNEL_A_PIN)
#define CHANNEL_A_LOW CHANNEL_A_PORT &= ~(1 << CHANNEL_A_PIN)
#define CHANNEL_A_HIGH CHANNEL_A_PORT |= (1 << CHANNEL_A_PIN)

void servo_init();
void servo_us(uint16_t us);
void servo_turn_off();
void servo_turn_on();
void pencil_lift();
void pencil_draw();
#endif
