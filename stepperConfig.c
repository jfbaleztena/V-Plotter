/*___________________________________________________________________________________________________

Title:
	stepperConfig.c
	
File version: 1.1
Project version: 1.1

Description:
	Part of stepperCon project.

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

//=============================================================================
// Include
//=============================================================================
#include "stepperConfig.h"

//=============================================================================
// Functions
//=============================================================================

/*-----------------------------------------------------------------------------
	Initialization function used to set the stepper motor pins.
	Because the number of stepper motors can vary, the user must define
	the pins and populate this function accordingly.
-------------------------------------------------------------------------------*/
void stepperSetupIO(void){
/* Test Pin/LED used for debugging and time measurements */ 
	T_PIN_DDR |= (1 << T_PIN);
	T_PIN_PORT &= ~(1 << T_PIN);
	
/* X Axis Motor */
	// ENABLE pin set as output high 
	// to disable the stepper driver module
	X_AXIS_EN_PORT |= (1 << X_AXIS_EN_PIN); // high
	X_AXIS_EN_DDR |= (1 << X_AXIS_EN_PIN); // output mode
	
	// STEP pin as output
	X_AXIS_STEP_DDR |= (1 << X_AXIS_STEP_PIN);
	// DIR pin as output
	X_AXIS_DIR_DDR |= (1 << X_AXIS_DIR_PIN);

	Y_AXIS_EN_PORT |= (1 << Y_AXIS_EN_PIN); // high
	Y_AXIS_EN_DDR |= (1 << Y_AXIS_EN_PIN); // output mode
	
	// STEP pin as output
	Y_AXIS_STEP_DDR |= (1 << Y_AXIS_STEP_PIN);
	// DIR pin as output
	Y_AXIS_DIR_DDR |= (1 << Y_AXIS_DIR_PIN);
}


/*-----------------------------------------------------------------------------
	STEPPER MOTOR related functions.
	Each stepper needs a group of 3 functions with the following roles: 
	- enabling/disabling the stepper driver module (EN pin)
	- setting the direction pin (DIR pin)
	- and most importantly stepping the motor (STEP pin)
	The functions can have any name.
	Functions can be empty if not used but they must be defined
	for when the motor is initialized.
-------------------------------------------------------------------------------*/

/*=============================================================================
	MOTOR 1 related functions
===============================================================================*/
/*-----------------------------------------------------------------------------
	ENABLE pin
-------------------------------------------------------------------------------*/
void motorX_EN(bool enable){
	// Replace the pin and/or name according to your project
	
	if(enable){
		X_AXIS_EN_PORT &= ~(1 << X_AXIS_EN_PIN); // pin low - enable module
	}else{
		X_AXIS_EN_PORT |= (1 << X_AXIS_EN_PIN); // pin high - disable module
	}
}

/*-----------------------------------------------------------------------------
	STEP pin
	// On a 16MHz device, setting a pin HIGH then LOW takes 188ns.
	// If a higher CPU frequency is used or the stepper driver expects
	// a longer pulse time, add a delay when changing the pin state.
-------------------------------------------------------------------------------*/
void motorX_STEP(void){
	// Replace the pin and/or name according to your project
	
	X_AXIS_STEP_PORT |= (1 << X_AXIS_STEP_PIN); // pin high
	
	// This delay can be commented out or it's duration modified.
	// The delay depends on the CPU clock and stepper driver module.
	// Some drivers such as A4988 need some time delay between pin
	// transition from high to low.
	// Some drivers like TMC2209 do not require them (at least on 16MHz clocks).
	// When using higher CPU speeds, consider adding a delay according to
	// the driver's datasheet.
	_delay_us(1);
	
	X_AXIS_STEP_PORT &= ~(1 << X_AXIS_STEP_PIN); // pin low
}

/*-----------------------------------------------------------------------------
	DIR pin
-------------------------------------------------------------------------------*/
void motorX_DIR(int8_t dir){
	// Replace the pin and/or name according to your project
	// DIRECTION_CCW an DIRECTION_CW are part of DIRECTION enum and should not be changed
	
	if(dir == DIRECTION_CCW){
		X_AXIS_DIR_PORT |= (1 << X_AXIS_DIR_PIN); // pin high
	}else{
		X_AXIS_DIR_PORT &= ~(1 << X_AXIS_DIR_PIN); // pin low
	}
}

/*=============================================================================
	To add more motors, simply copy/paste the above code, 
	and change function and pin names.
===============================================================================*/
/*-----------------------------------------------------------------------------
	ENABLE pin
-------------------------------------------------------------------------------*/
void motorY_EN(bool enable){
	// Replace the pin and/or name according to your project
	
	if(enable){
		Y_AXIS_EN_PORT &= ~(1 << Y_AXIS_EN_PIN);
	}else{
		Y_AXIS_EN_PORT |= (1 << Y_AXIS_EN_PIN);
	}
}

/*-----------------------------------------------------------------------------
	STEP pin
	// On a 16MHz device, setting a pin HIGH then LOW takes 188ns.
	// If a higher CPU frequency is used or the stepper driver expects
	// a longer pulse time, add a delay when changing the pin state.
-------------------------------------------------------------------------------*/
void motorY_STEP(void){
	// Replace the pin and/or name according to your project
	
	Y_AXIS_STEP_PORT |= (1 << Y_AXIS_STEP_PIN);
	
	// This delay can be commented out or it's duration modified.
	// The delay depends on the CPU clock and stepper driver module.
	// Some drivers such as A4988 need some time delay between pin
	// transition from high to low.
	// Some drivers like TMC2209 do not require them (at least on 16MHz clocks).
	// When using higher CPU speeds, consider adding a delay according to
	// the driver's datasheet.
	_delay_us(1);
	
	Y_AXIS_STEP_PORT &= ~(1 << Y_AXIS_STEP_PIN);
}

/*-----------------------------------------------------------------------------
	DIR pin
-------------------------------------------------------------------------------*/
void motorY_DIR(int8_t dir){
	// Replace the pin and/or name according to your project
	// DIRECTION_CCW an DIRECTION_CW are part of DIRECTION enum and should not be changed
	
	if(dir == DIRECTION_CW){
		Y_AXIS_DIR_PORT |= (1 << Y_AXIS_DIR_PIN); // pin high
	}else{
		Y_AXIS_DIR_PORT &= ~(1 << Y_AXIS_DIR_PIN); // pin low
	}
}