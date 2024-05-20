/*___________________________________________________________________________________________________

Title:
	stepperConfig.h
	
File version: 1.1
Project version: 1.1

Description:
	Part of stepperCon project.
	Used to define pins and custom user functions needed for controlling stepper motors,
	and other settings such as CPU clock and number of stepper motors used.
	
	For complete details visit
	https://www.programming-electronics-diy.xyz/2023/10/steppercon-library-for-controlling.html

Author:
 	Liviu Istrate
	istrateliviu24@yahoo.com
	www.programming-electronics-diy.xyz

Donate:
	Software development takes time and effort so if you find this useful consider a small donation at:
	https://www.paypal.com/donate/?hosted_button_id=L5ZL3SAN6NABY
_____________________________________________________________________________________________________*/


/* ----------------------------- LICENSE - GNU GPL v3 -----------------------------------------------

* This license must be included in any redistribution.

* Copyright (C) 2023 Liviu Istrate, www.programming-electronics-diy.xyz (istrateliviu24@yahoo.com)

* Project URL: https://www.programming-electronics-diy.xyz/2023/10/steppercon-library-for-controlling.html

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


#ifndef STEPPERCONFIG_H_
#define STEPPERCONFIG_H_

//=========================================================================================
// USER SETTINGS 1/3
//=========================================================================================
// F_CPU is defined inside micros.h
//#define F_CPU									16000000 // CPU clock frequency in Hertz

// Number of stepper motor drivers to control. Increasing this will increase code size.
#define MAX_NR_OF_STEPPER_DRIVERS				2

// Number of maximum coordinated steppers.
// Usually 3 for X, Y and Z axis.
#define NR_OF_SYNC_STPPERS						3

// Number of segments of speed profile to hold in a buffer.
// Normally this should not be modified. Increasing this number will
// increase memory usage and also the controlled motor stop 
// will have a delay since there will be more segments in the buffer
// at cruising speed before replaced with deceleration segments.
#define SEGMENT_BUFFER_SIZE						6 // [default: 6]


//=========================================================================================
// Include - must be after defining F_CPU, number of drivers and segment buffer size
//=========================================================================================
#include "stepperCon.h"

//=========================================================================================
// USER SETTINGS 2/3 - I/O pins

// Each stepper driver needs a group of 3 pins: 
// enable (EN), direction (DIR) and stepping (STEP)
// Remove or add pin groups according to your project.
// The name of the pins can be arbitrary.
// Each pin type consists of:
// - DDR (Data Direction Register) followed by the port letter. E.g: DDRB (for port B).
// - PORT followed by the port letter. E.g: PORTB (for port B).
// - Number of the pin on the port from 0 to 7 if the port has 8 pins.
//=========================================================================================

/* Test Pin/LED used for debugging and time measurements */ 
#define T_PIN_DDR						DDRD
#define T_PIN_PORT						PORTD
#define T_PIN							7

/*=============================================================================
	MOTOR 1 related pins
===============================================================================*/
/* X AXIS */
// Enable pin
#define X_AXIS_EN_DDR					DDRD
#define X_AXIS_EN_PORT					PORTD
#define X_AXIS_EN_PIN					8	

// Step pin
#define X_AXIS_STEP_DDR					DDRD
#define X_AXIS_STEP_PORT				PORTD
#define X_AXIS_STEP_PIN					2	

// Example for UPDI devices
//#define X_AXIS_STEP_DDR				PORTA.DIR
//#define X_AXIS_STEP_PORT				PORTA.OUT
//#define X_AXIS_STEP_PIN				PIN2_bp

// Direction pin
#define X_AXIS_DIR_DDR					DDRD
#define X_AXIS_DIR_PORT					PORTD
#define X_AXIS_DIR_PIN					5	

/*=============================================================================
	MOTOR 2 related pins
===============================================================================*/
/* Y AXIS */
// Enable pin
#define Y_AXIS_EN_DDR					DDRD
#define Y_AXIS_EN_PORT					PORTD
#define Y_AXIS_EN_PIN					8	

// Step pin
#define Y_AXIS_STEP_DDR					DDRD
#define Y_AXIS_STEP_PORT				PORTD
#define Y_AXIS_STEP_PIN					3	

// Direction pin
#define Y_AXIS_DIR_DDR					DDRD
#define Y_AXIS_DIR_PORT					PORTD
#define Y_AXIS_DIR_PIN					6	

//=========================================================================================
// USER SETTINGS 3/3 - Function Declarations

// Functions are explained and defined in the c file with the same name
//=========================================================================================
void stepperSetupIO(void);

/*=============================================================================
	MOTOR 1 related functions
===============================================================================*/
void motorX_EN(bool enable);
void motorX_STEP(void);
void motorX_DIR(int8_t dir);

/*=============================================================================
	MOTOR 2 related functions
===============================================================================*/
void motorY_EN(bool enable);
void motorY_STEP(void);
void motorY_DIR(int8_t dir);

#endif /* STEPPERCONFIG_H_ */