/*___________________________________________________________________________________________________

Title:
	micros.h v2.0

Description:
	Library for AVR microcontrollers that triggers an interrupt every n microseconds
	and counts them.
	
	For complete details visit
	https://www.programming-electronics-diy.xyz/2021/01/millis-and-micros-library-for-avr.html

Author:
 	Liviu Istrate
	istrateliviu24@yahoo.com
	www.programming-electronics-diy.xyz

Donate:
	Software development takes time and effort so if you find this useful consider a small donation at:
	paypal.me/alientransducer
_____________________________________________________________________________________________________*/


/* ----------------------------- LICENSE - GNU GPL v3 -----------------------------------------------

* This license must be included in any redistribution.

* Copyright (C) 2021 Liviu Istrate, www.programming-electronics-diy.xyz (istrateliviu24@yahoo.com)

* Project URL: https://www.programming-electronics-diy.xyz/2021/01/millis-and-micros-library-for-avr.html

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.

--------------------------------- END OF LICENSE --------------------------------------------------*/

/* ----------------------------- CHANGELOG ------------------------------------------------------------
	v2.0
		- added support for modern AVR (UPDI devices). Tested on tinyAVR402.

-----------------------------------------------------------------------------------------------------*/

#ifndef MICROS_H_
#define MICROS_H_

/*=============================================================================
	INCLUDE
==============================================================================*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>

/*=============================================================================
	SYSTEM CONSTANTS
==============================================================================*/
	#define AVR_CLASS			0 // Classic AVR
// #ifdef UCSR0A
// 	#define AVR_CLASS			0 // Classic AVR
// #else
// 	#define AVR_CLASS			1 // Modern AVR (UPDI devices)
// 	#include <avr/cpufunc.h>
// #endif

#define TIMING_USING_RTC		1
#define TIMING_USING_TCA		2

#define MICROS_TIMER0 			0
#define MICROS_TIMER1 			1
#define MICROS_TIMER2 			2
#define MICROS_TIMER3 			3
#define MICROS_TIMER4 			4

/*=============================================================================
	USER SETUP SECTION
==============================================================================*/

/*---------------------------------------------------------------
Only needed on non-UPDI devices. Needed to configure the timer.
For UPDI devices F_CPU is a function argument.

// F_CPU is best to be defined inside project properties in Microchip Studio
// or a Makefile if custom Makefiles are used.
// See https://www.programming-electronics-diy.xyz/2024/01/defining-fcpu.html
----------------------------------------------------------------*/
//#define F_CPU							16000000UL

/*---------------------------------------------------------------
Microseconds data type and how long it takes to overflow
	Data type				- Max time span			- Memory used
	unsigned char			- 255 microseconds		- 1 byte
	unsigned int			- 65.5 milliseconds		- 2 bytes
	unsigned long			- 71.58 minutes			- 4 bytes
	unsigned long long		- 584942 years		 	- 8 bytes
	
On 1MHz CPU it takes around 77uS to increment long long variable
----------------------------------------------------------------*/
typedef unsigned long micros_t;

/*-------------------------------------------------------------
Non-UPDI devices
	MICROS_TIMER0 - 8-bits
	MICROS_TIMER1 - 16-bits
	MICROS_TIMER2 - 8-bits
	MICROS_TIMER3 - 16-bits
	MICROS_TIMER4 - 16-bits
---------------------------------------------------------------*/
#define MICROS_TIMER 				MICROS_TIMER2 // which timer to use

/*--------------------------------------------------------------
Error calibration. A negative or positive number
that will be added or subtracted from timer register value.
----------------------------------------------------------------*/
#define MICROS_ERR_CAL				0 // usually 1 or -1 for RTC

/*--------------------------------------------------------------
UPDI devices
Select timing module. Only applies for UPDI devices.
	TIMING_USING_RTC - use internal RTC
	TIMING_USING_TCA - use timer TCA channel 0
----------------------------------------------------------------*/
#define MICROS_TIMING_MODULE		TIMING_USING_RTC

/*-------------------------------------------------------------
UPDI devices
RTC clock selection
	RTC_CLKSEL_INT32K_gc - Internal 32kHz OSC
	RTC_CLKSEL_INT1K_gc  - Internal 1kHz OSC
	RTC_CLKSEL_EXTCLK_gc - External Clock
	
At the moment only RTC_CLKSEL_INT32K_gc is implemented.
---------------------------------------------------------------*/
#define MICROS_RTC_CLOCK			RTC_CLKSEL_INT32K_gc


/*=============================================================================
	SYSTEM MACROS
==============================================================================*/
#if AVR_CLASS == 0
	#ifndef F_CPU
		#error "F_CPU not defined."
	#endif

#if MICROS_TIMER == MICROS_TIMER0

	// Timer0
	#if F_CPU > 1000000 // 1MHz - 20MHz
		#define CLOCKSEL 		(_BV(CS01))
		#define MICROS_PRESCALER 		8
		
	#elif F_CPU >= 125000 // 125KHz - 1MHz
		#define CLOCKSEL 		(_BV(CS00))
		#define MICROS_PRESCALER 		1
		
	#else
		#error "F_CPU out of range."
	#endif

	#define REG_TCCRA			TCCR0A
	#define REG_TCCRB			TCCR0B
	#define REG_TIMSK			TIMSK0
	#define REG_OCR				OCR0A
	#define BIT_WGM				WGM01
	#define BIT_OCIE			OCIE0A
	#define ISR_VECT			TIMER0_COMPA_vect
	#define pwr_enable()		power_timer0_enable()
	#define pwr_disable()		power_timer0_disable()

	#define SET_TCCRA()			(REG_TCCRA = _BV(BIT_WGM))
	#define SET_TCCRB()			(REG_TCCRB = CLOCKSEL)

#elif MICROS_TIMER == MICROS_TIMER1

	// Timer1
	// 125KHz - 20MHz
	#define CLOCKSEL 			(_BV(CS10))
	#define MICROS_PRESCALER 			1

	#define REG_TCCRA			TCCR1A
	#define REG_TCCRB			TCCR1B
	#define REG_TIMSK			TIMSK1
	#define REG_OCR				OCR1A
	#define BIT_WGM				WGM12
	#define BIT_OCIE			OCIE1A
	#define ISR_VECT			TIMER1_COMPA_vect
	#define pwr_enable()		power_timer1_enable()
	#define pwr_disable()		power_timer1_disable()

	#define SET_TCCRA()			(REG_TCCRA = 0)
	#define SET_TCCRB()			(REG_TCCRB = _BV(BIT_WGM) | CLOCKSEL)

#elif MICROS_TIMER == MICROS_TIMER2

	// Timer2
	#if F_CPU > 1000000 // 1MHz - 20MHz
		#define CLOCKSEL 		(_BV(CS21))
		#define MICROS_PRESCALER 		8
		
	#elif F_CPU >= 125000 // 125KHz - 1MHz
		#define CLOCKSEL 		(_BV(CS20))
		#define MICROS_PRESCALER 		1
		
	#else
		#error "F_CPU out of range."
	#endif

	#define REG_TCCRA			TCCR2A
	#define REG_TCCRB			TCCR2B
	#define REG_TIMSK			TIMSK2
	#define REG_OCR				OCR2A
	#define BIT_WGM				WGM21
	#define BIT_OCIE			OCIE2A
	#define ISR_VECT			TIMER2_COMPA_vect
	#define pwr_enable()		power_timer2_enable()
	#define pwr_disable()		power_timer2_disable()

	#define SET_TCCRA()			(REG_TCCRA = _BV(BIT_WGM))
	#define SET_TCCRB()			(REG_TCCRB = CLOCKSEL)
	
#elif MICROS_TIMER == MICROS_TIMER3

	// Timer3
	// 125KHz - 20MHz
	#define CLOCKSEL 			(_BV(CS30))
	#define MICROS_PRESCALER 			1

	#define REG_TCCRA			TCCR3A
	#define REG_TCCRB			TCCR3B
	#define REG_TIMSK			TIMSK3
	#define REG_OCR				OCR3A
	#define BIT_WGM				WGM32
	#define BIT_OCIE			OCIE3A
	#define ISR_VECT			TIMER3_COMPA_vect
	#define pwr_enable()		power_timer3_enable()
	#define pwr_disable()		power_timer3_disable()

	#define SET_TCCRA()			(REG_TCCRA = 0)
	#define SET_TCCRB()			(REG_TCCRB = _BV(BIT_WGM) | CLOCKSEL)
	
#elif MICROS_TIMER == MICROS_TIMER3

	// Timer4
	// 125KHz - 20MHz
	#define CLOCKSEL 			(_BV(CS40))
	#define MICROS_PRESCALER 			1

	#define REG_TCCRA			TCCR4A
	#define REG_TCCRB			TCCR4B
	#define REG_TIMSK			TIMSK4
	#define REG_OCR				OCR4A
	#define BIT_WGM				WGM42
	#define BIT_OCIE			OCIE4A
	#define ISR_VECT			TIMER4_COMPA_vect
	#define pwr_enable()		power_timer4_enable()
	#define pwr_disable()		power_timer4_disable()

	#define SET_TCCRA()			(REG_TCCRA = 0)
	#define SET_TCCRB()			(REG_TCCRB = _BV(BIT_WGM) | CLOCKSEL)

#else
	#error "Bad MICROS_TIMER set"
#endif
#endif


/*=============================================================================
	GLOBAL VARIABLES
==============================================================================*/
volatile micros_t microseconds;
uint16_t micros_resolution;


/*=============================================================================
	FUNCTION PROTOTYPES
==============================================================================*/
void micros_init(uint32_t f_cpu, uint16_t resolution);
micros_t micros_get(void);
void micros_resume(void);
void micros_pause(void);
void micros_reset(void);
void micros_add(micros_t us);
void micros_subtract(micros_t us);

#endif /* MICROS_H_ */