/*___________________________________________________________________________________________________

Title:
	stepperCon.h
	
File version: 1.1
Project version: 1.1

Description:
	Part of stepperCon project.
	stepperCon is a library used to control multiple stepper motor modules using STEP and DIR pins.
	It can achieve up to 25kHz stepping frequency on a 16MHz CPU by segmenting the speed profile.
	The algorithm is inspired by the GRBL project (https://github.com/grbl/grbl).
	The steps are executed by a timer interrupt	that triggers every 40us. Coordinating multiple
	motors is also possible by the use of Bresenham's algorithm.
	
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
* The algorithm used to generate the segmented speed profile 
* is inspired by the GRBL project (https://github.com/grbl/grbl).

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


/* ----------------------------- Changelog -----------------------------------------------
	v1.1
		- 2-2-2024: added support for UPDI devices
	v1.0:
		- Release date: 4, October, 2023

--------------------------------- END OF CHANGELOG --------------------------------------------------*/


#ifndef STEPCONTROLLER_H_
#define STEPCONTROLLER_H_

//=======================================================================================
// Include
//=======================================================================================
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
// Triggers a timer interrupt for step execution
#include "micros.h"
#include <util/delay.h>
// Used for debugging
//#include "uart.h"
#include "stepperConfig.h"

//=======================================================================================
// Settings
//=======================================================================================

// The temporal resolution of the acceleration management subsystem. A higher number gives smoother
// acceleration, particularly noticeable on machines that run at very high feed rates, but may negatively
// impact performance. The correct value for this parameter is machine dependent, so it's advised to
// set this only as high as needed.
// NOTE: Changing this value also changes the execution time of a segment in the step segment buffer.
// When increasing this value, this stores less overall time in the segment buffer and vice versa. Make
// certain the step segment buffer is increased/decreased to account for these changes.
// Set to 1000 or lower when using more than 3 motors.
#define ACCELERATION_TICKS_PER_SECOND		4000.0 // [default: 4000]

//=======================================================================================
// Constants and macros
//=======================================================================================

// Acceleration and deceleration are multiplied by this factor. This is so the user can 
// pass smaller numbers to the functions. This prevents user errors. For example if the
// acceleration needs to be 50000 it is more readable to pass 500 instead.
#define ACCEL_SCALING_FACTOR				100 // do not change [default: 100]

// Some useful constants
#define DT_SEGMENT			(1.0 / ACCELERATION_TICKS_PER_SECOND) // Maximum segment time (sec/segment)

// Useful macros
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

//=======================================================================================
// Enums
//=======================================================================================
// Symbolic names for the direction the motor is turning
// +1 and -1 values are used for faster increment/decrement of current position
typedef enum Direction{
	DIRECTION_CW  = 1,		// Clockwise
	DIRECTION_CCW = -1		// Counter-Clockwise
} Direction;

typedef enum{
	STOPPED = 0,
	ACCELERATION = 1,
	CRUISING = 2,
	DECELERATION = 3,
	SPEEDING_DOWN = 4
} MotorState;

//=======================================================================================
// Structures
//=======================================================================================

// Primary stepper segment ring buffer. Contains small, short line segments
// for the stepper algorithm to execute.
typedef struct {
	int16_t n_step;					// Number of step events to be executed for this segment
	int32_t step_duration;			// Duration of a single step
} segment_t;

// Main stepper motor structure
typedef struct{
	// Function pointers
	void (*EN_f)(bool enable);				// Pointer to a function that sets the ENABLE pin
	void (*DIR_f)(Direction dir);			// Pointer to a function that sets the DIRECTION pin
	void (*STEP_f)(void);					// Pointer to a function that pulses the STEP pin.
											// On a 16MHz device, setting a pin HIGH then LOW takes 188ns.
											// If a higher CPU frequency is used or the stepper driver expects
											// a longer pulse time, add a delay when changing the pin state.
	
	// Motor characteristics
	float currentDegreePos;					// Keeps track of current position in degrees (used by stepperToDegree function)
	uint16_t stepsPerRev;					// Steps per revolution (usually 200). For 1.8 degree motor (360 / 1.8 = 200).
	uint16_t stepDivision;					// Division factor applied by the stepper driver module
	
	// Variables used by the software driver
	float setSpeed;							// Desired motor speed in steps/second
	uint32_t accel;							// Acceleration in steps/second2
	uint32_t decel;							// Deceleration in steps/second2
	int32_t currentPosition;				// Current absolute motor position in steps
	int32_t current_step;					// Current step number executed by the ISR
	int32_t m;								// Number of steps necessary for current movement
	int32_t accelerate_until;				// Number of steps needed to accelerate to the desired speed (Equation 16)
	int32_t decel_value;					// Steps needed for deceleration
	uint32_t accel_limit;					// Number of steps after deceleration starts (disregarding desired speed)
	bool safeStop;							// Flag for safe deceleration
	bool movementDone;						// True if the current movement has been completed
	bool holdTorque;						// If true the stepper driver will remain enabled when motor is idle
	int8_t direction;						// Current direction of movement, used to keep track of position
	uint8_t motorState;						// State of the motor: Accelerating, Cruising, Decelerating, Stopped
											// Only STOPPED and ACCELERATION is implemented.
	
	// Variables used by the segment generator
	float seg_time_var;						// Segment time (float)
	float seg_current_speed;				// Speed in steps/second of the active generated segment (float)
	uint32_t seg_current_step;				// Current step number summing the steps in all segments
	uint8_t seg_motor_state;				// Logical state of the motor during segment creation: Accelerating, Cruising, Decelerating, Stopped
	int8_t steps_var;						// Number of steps above acceleration steps that were removed and must be added to deceleration
	float speed_buf;						// Speed buffered at the end of acceleration and restored before deceleration starts
	
	// Variables used by the ISR segment executor
	uint32_t ISR_step_duration;				// Step duration in us of active executed segment
	uint32_t ISR_lastStepTime;				// Previous time
	int16_t ISR_step_count;					// Number of steps in the executed segment (int16_t)
	bool ISR_load_segment;					// Flag that controls when the ISR can execute segments
	
	// Needed by Bresenham line algorithm
	bool is_main_axis;						// The motor with longest execution time is considered the main one
	uint8_t id;								// stepper array index used to sync the axis
	
	// Circular array buffer for active segments across the ramp profile
	segment_t segment_buffer[SEGMENT_BUFFER_SIZE];
	
	// Step segment ring buffer indices
	uint8_t segment_buffer_tail;
	uint8_t segment_buffer_head;
	uint8_t segment_buffer_ovf;
} stepperCon;

extern stepperCon stepper[];


// Used by the Bresenham line algorithm
typedef struct {
	// Counter variables for the Bresenham line tracer
	uint32_t counter_x,	counter_y, counter_z;
	
	uint32_t steps[NR_OF_SYNC_STPPERS];
	uint32_t step_event_count;
	
	// ID of steppers to sync
	uint8_t ids[NR_OF_SYNC_STPPERS];
	
	uint8_t main_axis_id;
	bool is_axis_enabled[NR_OF_SYNC_STPPERS];
} sync_t;

sync_t sync;

//=======================================================================================
// Function Prototypes
//=======================================================================================
// Initialization function
stepperCon* stepperAdd(uint16_t steps_per_revolution);

// Set parameters
void stepperSetStepDivision(stepperCon* self, uint8_t div);
void stepperHoldTorque(stepperCon* self, bool hold_torque);
void stepperSetSpeed(stepperCon* self, float speed);
void stepperSetAcceleration(stepperCon* self, uint32_t acceleration);
void stepperSetDeceleration(stepperCon* self, uint32_t deceleration);
void stepperSetCurrentPosition(stepperCon* self, int32_t position);
void stepperDefineXYZ(stepperCon* x, stepperCon* y, stepperCon* z);

// Get values
uint32_t stepperGetCurrentSpeed(stepperCon* self);
int32_t stepperGetCurrentPosition(stepperCon* self);
bool stepperIsIdle(stepperCon* self);
bool stepperIsIdleXYZ(void);

// Motion
void stepperRun(void);
void stepperMoveTo(stepperCon* self, int32_t absolute);
void stepperMove(stepperCon* self, int32_t relative);
void stepperMoveToXYZ(const int32_t xyz[]);

// Stopping
void stepperSafeStop(stepperCon* self);
void stepperEmergencyStop(stepperCon* self);

// Extra
void stepperFreeSpin(stepperCon* self, Direction dir);
void stepperToDegree(stepperCon* self, float degree);

#endif /* STEPCONTROLLER_H_ */