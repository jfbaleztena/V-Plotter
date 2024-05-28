#include "servo.h"
#include <util/delay.h>

bool servo_init()
{
	CHANNEL_A_OUT;
	CHANNEL_A_LOW;

	// Set Timer1 in Fast PWM mode (14)
	TCCR1A |= (1 << WGM11); // | (1 << WGM10);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);

	// Prescaler
	// TCCR1B |= (1 << CS11); // Set to 8

	// Set frequency
	ICR1 = FREQUENCY_OCR + F_CALLIBRATION;

	OCR1A = 0;

	TIMSK1 |= (1 << OCIE1A) | (1 << ICIE1);

	pencil_lift();
	return true;
}

void servo_turn_off(){
	TCCR1B &= ~(1 << CS11);
}

void servo_turn_on(){
	TCCR1B |= (1 << CS11); // Set to 8
}

void servo_us(uint16_t us)
{
	CHANNEL_A_REG = us * MULTIPLAYER;
}

ISR(TIMER1_COMPA_vect)
{
	CHANNEL_A_LOW;
}

ISR(TIMER1_CAPT_vect)
{
	CHANNEL_A_HIGH;
}

void pencil_draw(){
    servo_turn_on();
    servo_us(1000);
    _delay_ms(200);
    servo_turn_off();
}

void pencil_lift(){
    _delay_ms(2000);
    servo_turn_on();
    servo_us(250);
    _delay_ms(200);
    servo_turn_off();
}