/*___________________________________________________________________________________________________

Title:
	stepperCon.c
	
File version: 1.1
Project version: 1.1

Description:
	Part of stepperCon project.
	See steppercon.h

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

//=============================================================================
// Include
//=============================================================================
#include "stepperCon.h"

//=============================================================================
// Globals
//=============================================================================
stepperCon stepper[MAX_NR_OF_STEPPER_DRIVERS];
uint8_t nr_of_stepper_drivers; // number of stepper drivers to control
//static sync_t sync;

//=============================================================================
// Private Function Declaration
//=============================================================================
static void stepperInit(stepperCon* self);
static void stepperSetRampParameters(stepperCon* self, float speed);
static void stepperReset(stepperCon* self);
static void _stepperSafeStop(stepperCon* self);

//=============================================================================
// Functions
//=============================================================================

/*-----------------------------------------------------------------------------
	Private function
	
	Initialize stepperCon and segment buffer structures values to 0.
	Set some initial values.
-------------------------------------------------------------------------------*/
static void stepperInit(stepperCon* self){
	memset(self, 0, sizeof(stepperCon)); // set initial values to 0
	memset(self->segment_buffer, 0, sizeof(self->segment_buffer)); // set initial values to 0
	
	self->movementDone = true;
	self->motorState = STOPPED;
	self->accel = (uint32_t)100 * ACCEL_SCALING_FACTOR;
	self->decel = (uint32_t)100 * ACCEL_SCALING_FACTOR;
	self->setSpeed = 200;
	self->stepDivision = 1;
}


/*-----------------------------------------------------------------------------
	Private function
	
	Calculates characteristics of the speed profile based on the given speed
	such as:
	- number of steps needed to accelerate to the desired speed
	- number of steps after deceleration starts (disregarding desired speed)
	- number of steps needed for deceleration
	- ramp type: triangle or trapezoidal
	
	Documentation:
	https://www.embedded.com/generate-stepper-motor-speed-profiles-in-real-time/
	http://ww1.microchip.com/downloads/en/AppNotes/doc8017.pdf
-------------------------------------------------------------------------------*/
static void stepperSetRampParameters(stepperCon* self, float speed){
	
	// Number of steps needed to accelerate to the desired speed (Equation 16).
	// Current step on the segmented ramp is added for when the speed is changed during motion.
	float accel_until = ((speed * speed) / (2 * self->accel)) + self->seg_current_step; // max_s_limit
	self->accelerate_until = accel_until;
		
	// Number of steps after deceleration starts (disregarding desired speed)
	self->accel_limit = (self->m * (uint64_t)self->decel) / (self->accel + self->decel);
	
	// Calculate steps needed for deceleration based on whether the accel and decel intersects or not
	if(self->accelerate_until < self->accel_limit){
		// Check if speed has been changed during motion and recalculate number of steps
		// needed for acceleration based on top speed not only the difference
		if(self->seg_current_step){
			accel_until = (self->setSpeed * self->setSpeed) / (2 * self->accel);
		}
		
		self->decel_value = accel_until * ((float)self->accel / self->decel);
	}else{
		self->decel_value = self->m - self->accel_limit;
		self->accelerate_until = self->accel_limit; // not enough steps to reach full acceleration
	}
}


/*-----------------------------------------------------------------------------
	Private function
	
	Resets some variables before each movement
-------------------------------------------------------------------------------*/
static void stepperReset(stepperCon* self){
	//self->ISR_exec_segment = NULL;
	self->segment_buffer_tail = 0;
	self->segment_buffer_head = 0; // empty = tail
	self->segment_buffer_ovf = false;
	self->seg_time_var = DT_SEGMENT;
	self->seg_current_step = 0;
	self->seg_current_speed = 0;
	//self->seg_dt = 0;
	self->motorState = ACCELERATION; // initialize as acceleration ramp
	self->seg_motor_state = ACCELERATION; // initialize as acceleration ramp
	self->current_step = 0;
}


/*-----------------------------------------------------------------------------
	Create a driver instance and returns a pointer to the created driver. 
	When number of drivers added equals	MAX_NR_OF_STEPPER_DRIVERS, 
	the return is NULL.
	
	steps_per_revolution - steps per revolution (usually 200). For 1.8 degree 
						   motor (360 / 1.8 = 200). Used to calculate RPM
						   and angle position. Must be used together with
						   stepperSetStepDivision().
-------------------------------------------------------------------------------*/
stepperCon* stepperAdd(uint16_t steps_per_revolution){
	if(nr_of_stepper_drivers >= MAX_NR_OF_STEPPER_DRIVERS) return NULL; // no room for more
	
	stepperCon* driver = &stepper[nr_of_stepper_drivers]; // pointer to stepper driver
	
	// Initialize stepper values
	stepperInit(driver);
	
	// Set the ID
	driver->id = nr_of_stepper_drivers;
	
	// Steps per revolution
	driver->stepsPerRev = steps_per_revolution;
	
	nr_of_stepper_drivers++; // this must be done last if the run function is inside the interrupt
	return driver;
}


/*-----------------------------------------------------------------------------
	Set division factor applied by the stepper driver module.
	
	self - pointer to a stepperCon structure object
	
	div - division factor. E.g: 8, 16, 32, 64
-------------------------------------------------------------------------------*/
void stepperSetStepDivision(stepperCon* self, uint8_t div){
	self->stepDivision = div;
}


/*-----------------------------------------------------------------------------
	self - pointer to a stepperCon structure object
	
	hold_torque - if set to true the enable pin will be used to keep the coils
				  energized while the motor is idle
-------------------------------------------------------------------------------*/
void stepperHoldTorque(stepperCon* self, bool hold_torque){
	self->holdTorque = hold_torque;
}


/*-----------------------------------------------------------------------------
	Sets the speed in steps per second at which the motor should run.
	The set speed can only be reached if there is enough steps (distance),
	if not try increasing the acceleration. The maximum speed can be reached 
	when the speed profile is trapezoid.
	
	Segments already in buffer will still run at previous speed, so a bigger buffer
	will lead to a greater delay in applying the new speed to the motor.
-------------------------------------------------------------------------------*/
void stepperSetSpeed(stepperCon* self, float speed){
	if((self->seg_motor_state == DECELERATION) || (speed == 0.0)) return;
	self->setSpeed = speed;
	uint32_t accel;
	
	// The speed adjustment is done to the prepared segmented ramp.
	// Segments already in buffer will not be affected so a bigger buffer
	// will lead to a greater delay in applying the new speed to the motor.
	if(speed >= self->seg_current_speed){
		// Speeding up		
		self->seg_motor_state = ACCELERATION;
		speed -= self->seg_current_speed; // speed difference
		accel = self->accel;
	}else{
		// Speeding down
		self->seg_motor_state = SPEEDING_DOWN;
		accel = self->decel;
	}
	
	// Re-calculate ramp parameters
	stepperSetRampParameters(self, speed);
	
	// Adjust time_var according to speed
	float speed_var = 0.0;
	float n_step_f = 0.0; // steps for one segment
	float current_speed = 0.0;
	self->seg_time_var = DT_SEGMENT;
	
	while(n_step_f < 1.0){
		speed_var = accel * self->seg_time_var;
		current_speed += speed_var;
		n_step_f = self->seg_time_var * (current_speed + 0.5 * speed_var);
		self->seg_time_var += DT_SEGMENT;
	}
}


/*-----------------------------------------------------------------------------
	Set acceleration and deceleration rate in steps/second2.
	The given values will be multiplied by ACCEL_SCALING_FACTOR(100) 
	so the user can use a lower number for readability. So instead of using
	15000 use 150.
	The deceleration will also be set here with the acceleration value, 
	so in case that they have the same value, running stepperSetDeceleration()
	is not necessary.
-------------------------------------------------------------------------------*/
void stepperSetAcceleration(stepperCon* self, uint32_t acceleration){
	// Acceleration cannot be zero when calculating ramp parameters
	// so assign a high value that will result in very few steps needed for acceleration
	// which is practically instant acceleration and with very low speeds it should pose no issue.
	if(acceleration == 0) acceleration = 5000;
	
	// Multiply acceleration by ACCEL_SCALING_FACTOR(100) so the user can use a lower number for readability
	self->accel = acceleration * ACCEL_SCALING_FACTOR;
	self->decel = self->accel;
	stepperSetRampParameters(self, self->setSpeed);
}


/*-----------------------------------------------------------------------------
	Similar to stepperSetAcceleration() this sets deceleration rate.
	When deceleration is the save as acceleration, this function is not required.
-------------------------------------------------------------------------------*/
void stepperSetDeceleration(stepperCon* self, uint32_t deceleration){
	self->decel = deceleration * ACCEL_SCALING_FACTOR;
	stepperSetRampParameters(self, self->setSpeed);
}


/*-----------------------------------------------------------------------------
	Used to change the current position with a new value, usually needed
	to reset the position to 0. The motor must be idle.
-------------------------------------------------------------------------------*/
void stepperSetCurrentPosition(stepperCon* self, int32_t position){
	// Set only if motor is not running
	if(self->movementDone == false) return;
	self->currentPosition = position;
}


/*-----------------------------------------------------------------------------
	Defines a group of up to three coordinated motors. Each parameter is 
	a pointer to a motor that should be in the group. Use NULL instead of a
	pointer when a slot in the group is not used.
-------------------------------------------------------------------------------*/
void stepperDefineXYZ(stepperCon* x, stepperCon* y, stepperCon* z){
	// Store the ID for each axis if not NULL
	if(x){
		sync.ids[0] = x->id;
		sync.is_axis_enabled[0] = true;
	}
	
	if(y){
		sync.ids[1] = y->id;
		sync.is_axis_enabled[1] = true;
	}
	
	if(z){
		sync.ids[2] = z->id;
		sync.is_axis_enabled[2] = true;
	}
}


/*-----------------------------------------------------------------------------
	Returns the current motor speed in steps per second
-------------------------------------------------------------------------------*/
uint32_t stepperGetCurrentSpeed(stepperCon* self){
	return 1000000 / self->ISR_step_duration; // convert step interval to steps per second
}


/*-----------------------------------------------------------------------------
	Returns the current absolute motor position in steps. 
	The returned value can be positive or negative.
-------------------------------------------------------------------------------*/
int32_t stepperGetCurrentPosition(stepperCon* self){
	return self->currentPosition;
}


/*-----------------------------------------------------------------------------
	Returns True if the motor is currently stopped or False if is running.
	Useful to trigger another movement after another one ends.
-------------------------------------------------------------------------------*/
bool stepperIsIdle(stepperCon* self){
	return (self->movementDone);
}


/*-----------------------------------------------------------------------------
	Returns True if the main motor in the coordinated group is currently 
	stopped or False if is running.
	Useful to trigger another movement after another one ends.
-------------------------------------------------------------------------------*/
bool stepperIsIdleXYZ(void){
	return (stepper[sync.main_axis_id].movementDone);
}


/*-----------------------------------------------------------------------------
	Move the motor using absolute positioning. The stepping is not done here.
	The function sets the logical and pin direction and calculates the number
	of steps necessary to reach the target position then sets a flag that triggers
	the segment generation function that in turn triggers the ISR where the 
	stepping is done.
	
	This function will not do anything if:
	- the motor is still running
	- the current position and target position are the same
	- the set speed is 0
-------------------------------------------------------------------------------*/
void stepperMoveTo(stepperCon* self, int32_t absolute){
	// Return if target position is the same as current position
	// or the motor is still moving or set speed is 0
	if((self->movementDone == false) || (self->currentPosition == absolute) || (self->setSpeed == 0)) return;
	
	// Set direction here to free up the ISR
	if(absolute < self->currentPosition){
		self->direction = DIRECTION_CCW;
	}else{
		self->direction = DIRECTION_CW;
	}
	
	self->DIR_f(self->direction); // set direction pin
	
	// Calculate number of steps needed to reach desired position and save the value in the m variable
	int32_t remainingSteps = absolute - self->currentPosition;
	//self->targetPosition = self->currentPosition + self->remainingSteps;
	if(remainingSteps < 0) remainingSteps = -remainingSteps;
	self->m = remainingSteps;
	
	stepperReset(self);  // reset must run first
	
	// Calculate steps needed for acceleration and when the deceleration has to starts
	stepperSetRampParameters(self, self->setSpeed);
	
	// Flag that triggers the movement
	self->movementDone = false;
}


/*-----------------------------------------------------------------------------
	Move the motor using relative positioning.
	See stepperMoveTo()
-------------------------------------------------------------------------------*/
void stepperMove(stepperCon* self, int32_t relative){
	stepperMoveTo(self, self->currentPosition + relative);
}


/*-----------------------------------------------------------------------------
	Move the group of coordinated motors using absolute positioning.
	The coordination is done using Bresenham line algorithm.
	Direction pin is also set here.
	Each driver is activated using the enable pin.
	
	This function will not do anything if:
	- the main motor in the group is still running
	- the current position and target position are the same
-------------------------------------------------------------------------------*/
void stepperMoveToXYZ(const int32_t xyz[]){
	int32_t distance = 0;
	uint8_t main_axis = 0;
	
	if(stepper[sync.main_axis_id].movementDone == false) return;
	sync.step_event_count = 0; // reset to 0
	
	// Find the main axis (the slowest) based on number of steps
	for(uint8_t i = 0; i < NR_OF_SYNC_STPPERS; i++){
		// Skip unused axes
		if(sync.is_axis_enabled[i] != true) continue;
		stepper[sync.ids[i]].is_main_axis = false; // reset to false
		
		// Find distance to travel
		distance = xyz[i] - stepper[sync.ids[i]].currentPosition;
		if(distance < 0) distance = -distance; // make it positive
		
		// Store the largest value
		if(distance > sync.step_event_count){
			sync.step_event_count = distance;
			main_axis = i;
		}
	}
	
	// Return if current position is the same as target position
	if(sync.step_event_count == 0) return;
	
	sync.main_axis_id = sync.ids[main_axis];
	stepper[sync.ids[main_axis]].is_main_axis = true; // flag main axis
	//stepper[sync.ids[main_axis]].targetPosition = xyz[main_axis];
	
	// Initialize Bresenham line and distance counters
	for(uint8_t i = 0; i < NR_OF_SYNC_STPPERS; i++){
		sync.steps[i] = 0;
		if(sync.is_axis_enabled[i] != true) continue;
		
		// Set direction
		if(xyz[i] < stepper[sync.ids[i]].currentPosition){
			stepper[sync.ids[i]].direction = DIRECTION_CCW;
		}else{
			stepper[sync.ids[i]].direction = DIRECTION_CW;
		}
		
		distance = xyz[i] - stepper[sync.ids[i]].currentPosition;
		if(distance < 0) distance = -distance;
		if(distance == 0) continue;
		sync.steps[i] = distance;
		
		stepper[sync.ids[i]].DIR_f(stepper[sync.ids[i]].direction); // set direction pin
		stepper[sync.ids[i]].EN_f(true); // enable driver
	}
	
	// >> 1 divides by 2 which is 10us faster than 0.5 or / 2
	sync.counter_x = sync.counter_y = sync.counter_z = sync.step_event_count >> 1; // divide by 2
	
	distance = xyz[main_axis] + 1;
	if(xyz[main_axis] < 0) distance -= 1;
	stepper[sync.main_axis_id].m = xyz[main_axis] - stepper[sync.main_axis_id].currentPosition;
	
	if(stepper[sync.main_axis_id].m < 0) stepper[sync.main_axis_id].m = -stepper[sync.main_axis_id].m;
	
	// Reset vars
	stepperReset(&stepper[sync.main_axis_id]);
	
	// Calculate steps needed for acceleration and when the deceleration has to starts
	stepperSetRampParameters(&stepper[sync.main_axis_id], stepper[sync.main_axis_id].setSpeed);
	
	// Trigger movement for main axis
	stepper[sync.main_axis_id].movementDone = false;
}


/*-----------------------------------------------------------------------------
	Stops the motor by triggering the deceleration.
	Segments already in the buffer will still be executed and only after that 
	the deceleration will start. Is difficult to to otherwise. The segment buffer
	shouldn't be too large as to affect deceleration distance/time.
	
	This function can be used inside an ISR, triggered by a limit switch for example,
	since it is only used to set up a flag. The actual deceleration is done
	by stepperRun().
-------------------------------------------------------------------------------*/
void stepperSafeStop(stepperCon* self){
	self->safeStop = true;
}


/*-----------------------------------------------------------------------------
	Private function
	
	See stepperSafeStop()
-------------------------------------------------------------------------------*/
static void _stepperSafeStop(stepperCon* self){
	self->safeStop = false;
	
	if(self->seg_motor_state == DECELERATION) return;
	
	// Find number of steps for acceleration depending on current ramp state: acceleration or cruising
	int32_t accel_until = ((self->seg_current_speed * self->seg_current_speed) / (2 * self->accel));
	
	// No need to accelerate further. Stop at current step.
	if(self->seg_motor_state == ACCELERATION) accel_until = self->seg_current_step;
	
	// Calculate steps needed for deceleration
	self->decel_value = accel_until * ((float)self->accel / self->decel);
	self->m = self->seg_current_step + self->decel_value;
	self->seg_motor_state = DECELERATION;
}


/*-----------------------------------------------------------------------------
	This function stops the motor instantly without deceleration by setting
	the enable pin to false, so use it with caution.
	
	Useful in emergency situations as the name suggests. Care should be taken
	at higher speeds or when the moved mass is heavy and a sudden stop could
	have a detrimental effect.
-------------------------------------------------------------------------------*/
void stepperEmergencyStop(stepperCon* self){
	self->EN_f(false);
	self->movementDone = true;
	self->ISR_load_segment = false;
	self->motorState = STOPPED;
}


/*-----------------------------------------------------------------------------
	Starts the motor and keeps it spinning for 2147483647 steps.
	Depending on the step rate this means a long time (days).
	The speed is set by the appropriate function.
	
	dir - direction of spin (DIRECTION_CW or DIRECTION_CCW)
-------------------------------------------------------------------------------*/
void stepperFreeSpin(stepperCon* self, Direction dir){
	if(self->movementDone == false) return;
	
	// Set number of steps to the maximum value the parameter can hold.
	// For int32 that is 2147483647.
	// ~0 -> setting all bits to one
	// >> 1 -> erasing sign bit, by shifting all bits to the right by one position
	// (unsigned int) typecasting to unsigned int after bits inversion instead of using ~0U, 
	// because C doesn't have a suffix for short,char literals (everything smaller than int in general).
	// Source: https://stackoverflow.com/a/32932520
	int32_t steps = (uint32_t)~0 >> 1;
	if(dir == DIRECTION_CCW) steps = 0 - steps; // make number negative
	
	stepperMoveTo(self, steps);
}


/*-----------------------------------------------------------------------------
	Useful to position the motor at a certain degree from 0 to 360 by finding
	the shortest path.
-------------------------------------------------------------------------------*/
void stepperToDegree(stepperCon* self, float degree){
	if(self->movementDone == false) return;
	
	float steps = 0.0;

	// How many degrees for 1 step
	float degrees_per_step = 360.0 / ((uint16_t)self->stepsPerRev * self->stepDivision);
	
	// Degrees in clockwise direction
	float a = (degree - self->currentDegreePos);
	
	// Degrees in counter-clockwise direction
	float b = 360 - fabs(degree - self->currentDegreePos);
	
	// Select shortest path
	if(fabs(a) <= b){
		steps = a; // CW
	}else{
		if(a > 0) steps = 0.0 - b; // CCW - make the value negative to set CCW direction
		else steps = b;
	}
	
	steps = (steps / degrees_per_step);
	self->currentDegreePos = degree; // save current position
	stepperMove(self, steps);
}


/*-----------------------------------------------------------------------------
	Prepares step segment buffer. Continuously called from main program.
	
	This is the main function that keeps the motors spinning together 
	with the ISR. The algorithm segments the speed profile and stores them
	inside the segment buffer. Each segment includes the number of steps
	and step duration. Lower speeds have a lower number of steps for each segment
	and higher speeds a higher number of steps per each segment. Even if at higher
	speeds the step duration is lower (higher frequency) the number of steps is
	also higher so the CPU has time to generate new segments for the ISR.
	If that is not the case, decrease ACCELERATION_TICKS_PER_SECOND to increase
	the number of steps per segment at higher speeds.
	
	The function should be placed inside the main function and ideally run 
	as often as possible without much delay because as soon as the ISR finishes
	executing segments, this function should be able to generate new ones until
	the desired position is reached.
	
	When the motor is idle the function will exit quicker. A new movement is
	triggered after a move function is used.
	
	The state of the segment generation (acceleration, cruising, deceleration)
	is out of phase (ahead) of the actual state of the motor and are independent.
	So for example during segment generation the state could be deceleration
	but the actual state of the motor could be acceleration or cruising.
	
	The motor is stopped after the current step (incremented by the ISR)
	is equal or greater than the number of steps to travel. Drivers are disabled
	using the enable pin if holding torque is false.
-------------------------------------------------------------------------------*/
void stepperRun(void){
	
	// Iterate for each motor driver
	for(uint8_t i = 0; i < nr_of_stepper_drivers; i++){
		
		// Reference for current motor to increase the access time
		stepperCon *motor = &stepper[i];
	
		// Check if the motor is stopped. The move() will set the movementDone 
		// flag that will trigger segment generation.
		if(motor->movementDone == true) continue;
		
		if(motor->current_step >= motor->m){
			motor->movementDone = true;
			motor->ISR_load_segment = false;
			motor->ISR_step_duration = 0;
			motor->ISR_step_count = 0;
			motor->motorState = STOPPED;
			motor->seg_motor_state = STOPPED;
			
			if(motor->holdTorque == false){
				if(motor->is_main_axis == true){
					stepper[sync.ids[0]].EN_f(false);
					stepper[sync.ids[1]].EN_f(false);
					stepper[sync.ids[2]].EN_f(false);
					motor->is_main_axis = false;
				}else{
					motor->EN_f(false);
				}
			}
			
			continue;
		}

		//bool activate_isr = false;
		
		// Check if number of steps in segments is less than target number
		while(motor->seg_current_step < motor->m){
			if(motor->safeStop == true){
				_stepperSafeStop(motor);
			}
			
			// When the tail is in front of the head this will prevent overwriting the data.
			// The overflow flag will be cleared by the consumer/reader after wrapping back to zero
			// and after that the producer/writer can write until the buffer is full, wrap back to zero and write until
			// the head is 2 places lower than the tail.
			if(motor->segment_buffer_ovf == true && motor->segment_buffer_head >= motor->segment_buffer_tail - 1) break;
		
			segment_t *prep_segment = &motor->segment_buffer[motor->segment_buffer_head]; // Initialize new segment
			float speed_var = 0.0;
			float n_step_f = 0.0; // steps for one segment (float)
			float seg_dt = 0.0;
			int32_t tmp_var = 0;
			uint32_t current_step_buf;
				
			do{
				switch(motor->seg_motor_state){
					case ACCELERATION:
						speed_var = motor->accel * motor->seg_time_var;
						
						// Calculate number of steps for this segment (float value)
						n_step_f = motor->seg_time_var * (motor->seg_current_speed + 0.5 * speed_var);
						
						// Integer part of steps for this segment
						prep_segment->n_step = n_step_f + 0.5; // round up
						
						// Current step number on the ramp
						motor->seg_current_step += prep_segment->n_step;
						
						// Add segment time to total computed ramp time
						seg_dt = motor->seg_current_step / motor->seg_current_speed;
						
						// Calculate step duration for this segment
						prep_segment->step_duration = (seg_dt / motor->seg_current_step) * 1000000;
						
						// End of acceleration
						if(motor->seg_current_step >= motor->accelerate_until && prep_segment->n_step > 0){
							// Clamping steps above acceleration value
							prep_segment->n_step -= motor->seg_current_step - motor->accelerate_until;
							motor->seg_current_step = motor->accelerate_until;
							motor->steps_var = prep_segment->n_step - 1;
							
							if(motor->accelerate_until < motor->accel_limit){
								motor->seg_motor_state = CRUISING;
								
								// Store the speed before cruising starts
								motor->speed_buf = motor->setSpeed - motor->seg_current_speed;
								// Set cruising speed
								motor->seg_current_speed = motor->setSpeed;
								
							}else{
								motor->seg_motor_state = DECELERATION;
								speed_var = motor->decel * motor->seg_time_var;
								motor->seg_current_speed -= speed_var;
							}
							
						}else{
							motor->seg_current_speed += speed_var;
						}
						
						if(n_step_f < 1){
							motor->seg_time_var += DT_SEGMENT;
						}
					break;
					
					case CRUISING:
						
						//n_step_f = 1; // prevents time_var from changing
						current_step_buf = motor->seg_current_step;
						motor->seg_current_step = current_step_buf + motor->setSpeed * motor->seg_time_var + 0.5;
						prep_segment->n_step = motor->seg_current_step - current_step_buf;
						
						// Add segment time to total computed ramp time
						seg_dt = motor->seg_current_step / motor->seg_current_speed;
						
						// Calculate step duration for this segment
						prep_segment->step_duration = (seg_dt / motor->seg_current_step) * 1000000;
						
						if(prep_segment->n_step < 1){
							motor->seg_time_var += DT_SEGMENT;
						}
						
						// End of cruising
						if(motor->seg_current_step >= motor->m - motor->decel_value){
							motor->seg_motor_state = DECELERATION;
							
							// Number of steps over step at which deceleration starts
							tmp_var = motor->seg_current_step - (motor->m - motor->decel_value);
								
							// Clamp cruising distance
							prep_segment->n_step -= tmp_var;
							motor->seg_current_step -= tmp_var;
							
							// Restore the speed before cruising started
							speed_var = motor->decel * motor->seg_time_var;
							motor->seg_current_speed -= speed_var;
							motor->seg_current_speed -= motor->speed_buf;
						}
					break;
					
					case SPEEDING_DOWN:
					case DECELERATION:
						speed_var = motor->decel * motor->seg_time_var;
						
						// Calculate number of steps for this segment (float value)
						n_step_f = motor->seg_time_var * (motor->seg_current_speed + 0.5 * speed_var);
						
						// Integer part of steps for this segment
						prep_segment->n_step = n_step_f + 0.5; // round up
						if(prep_segment->n_step <= 0) prep_segment->n_step = 1;
						
						if(motor->steps_var){
							prep_segment->n_step++;
							motor->steps_var--;
						}
						
						// Current step number on the ramp
						motor->seg_current_step += prep_segment->n_step;
						
						// Clamp number of steps in this segment if total steps are more than requested
						if(motor->seg_current_step > motor->m){
							prep_segment->n_step -= motor->seg_current_step - motor->m;
							motor->seg_current_step = motor->m;
							break;
						}
						
						// Add segment time to total computed ramp time
						seg_dt = motor->seg_current_step / motor->seg_current_speed;
						
						// Calculate step duration for this segment
						prep_segment->step_duration = (seg_dt / motor->seg_current_step) * 1000000;
						
						motor->seg_current_speed -= speed_var;
						
						if(motor->seg_motor_state == SPEEDING_DOWN && motor->seg_current_speed <= motor->setSpeed){
							motor->seg_motor_state = CRUISING;
							motor->seg_current_speed = motor->setSpeed;
						}
						
						// Increase segment time to ensure at least one step in segment.
						// Do not remove the if condition even if it is in the while loop also.
						if(n_step_f < 1){
							motor->seg_time_var -= DT_SEGMENT;
						}

				} // end switch
				
			}while(prep_segment->n_step < 1); // Exit do while loop. Segment complete.

			
			// Segment complete! Increment segment buffer indices.
			// Head index is first checked then changed so it can have the right
			// value if the ISR reads it during that time.
			if(motor->segment_buffer_head < SEGMENT_BUFFER_SIZE - 1){
				motor->segment_buffer_head++;
			}else{
				motor->segment_buffer_head = 0;
				motor->segment_buffer_ovf = true;
			}
			
			//activate_isr = true;
			motor->EN_f(true); // enable the driver
			// Flag for the ISR to start segment execution
			motor->ISR_load_segment = true;
			
		} // END while loop. Profile complete.
	
		// Flag for the ISR to start segment execution
		/*if(activate_isr){
			motor->EN_f(true); // enable the driver
			motor->ISR_load_segment = true;
		}*/

	} // END drivers loop
}



//-----------------------------------------------------------------------------
// ISR Handlers
//-----------------------------------------------------------------------------

#if AVR_CLASS == 0	// non-UPDI
ISR(ISR_VECT){

#else				// UPDI

#if MICROS_TIMING_MODULE == TIMING_USING_TCA
ISR(TCA0_CMP0_vect){
#elif MICROS_TIMING_MODULE == TIMING_USING_RTC
ISR(RTC_CNT_vect){
#endif
			
	#if MICROS_TIMING_MODULE == TIMING_USING_TCA
		TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm;	// clear channel 0 compare flag
	#elif MICROS_TIMING_MODULE == TIMING_USING_RTC
		RTC.INTFLAGS = RTC_OVF_bm;					// clear overflow flag
	#endif
#endif
	// Test pin used to measure execution time
	//T_PIN_PORT |= (1 << T_PIN);
	
	microseconds += micros_resolution;
	
	// Iterate for each motor driver
	for(uint8_t i = 0; i < nr_of_stepper_drivers; i++){
		// Create a reference for current motor to decrease the access time
		stepperCon *motor = &stepper[i];
		if(motor->ISR_load_segment != true) continue;
	
		// Casting to unsigned type is necessary to avoid negative numbers after micros overflow
		if((micros_t)(microseconds - motor->ISR_lastStepTime) >= motor->ISR_step_duration){
			motor->ISR_lastStepTime = microseconds;
			
			// If there is no step segment, attempt to pop one from the stepper buffer
			if(motor->ISR_step_count == 0){
				// Anything in the buffer? If so, load and initialize next step segment.
				if((motor->segment_buffer_tail != motor->segment_buffer_head) || (motor->segment_buffer_ovf == true)){
					// Initialize step segment timing per step and load number of steps to execute
					motor->ISR_step_count = motor->segment_buffer[motor->segment_buffer_tail].n_step;
					motor->ISR_step_duration = motor->segment_buffer[motor->segment_buffer_tail].step_duration;

				}else{
					// Test pin used to check when the segment generator didn't have time
					// to create new segments. Reaching here before the target position
					// is reached is not desired. Decrease ACCELERATION_TICKS_PER_SECOND
					// or increase interrupt time interval.
					//T_PIN_PORT |= (1 << T_PIN);
					//T_PIN_PORT &= ~(1 << T_PIN);
					
					motor->ISR_load_segment = false;
					continue;
				}
			}
			
			
			// Execute step displacement profile by Bresenham line algorithm.
			// If this is the main axis then step the secondary axes as well.
			if(motor->is_main_axis == true){
				// X axis
				if(sync.is_axis_enabled[0]){
					sync.counter_x += sync.steps[0];

					if(sync.counter_x > sync.step_event_count){
						stepper[sync.ids[0]].STEP_f();
						stepper[sync.ids[0]].currentPosition += stepper[sync.ids[0]].direction;
						stepper[sync.ids[0]].current_step++;
						sync.counter_x -= sync.step_event_count;
					}
				}
				
				// Y axis
				if(sync.is_axis_enabled[1]){
					sync.counter_y += sync.steps[1];

					if(sync.counter_y > sync.step_event_count){
						stepper[sync.ids[1]].STEP_f();
						stepper[sync.ids[1]].currentPosition += stepper[sync.ids[1]].direction;
						stepper[sync.ids[1]].current_step++;
						sync.counter_y -= sync.step_event_count;
					}
				}
				
				// Z axis
				if(sync.is_axis_enabled[2]){
					sync.counter_z += sync.steps[2];

					if(sync.counter_z > sync.step_event_count){
						stepper[sync.ids[2]].STEP_f();
						stepper[sync.ids[2]].currentPosition += stepper[sync.ids[2]].direction;
						stepper[sync.ids[2]].current_step++;
						sync.counter_z -= sync.step_event_count;
					}
				}
				
			}else if(motor->ISR_step_count){
				// Not a synchronized motor
				// Then pulse the stepping pins
				motor->STEP_f();
				
				// Keep track of absolute current position.
				// When direction is 1 (clockwise) the current position will be incremented by 1.
				// When direction is -1 (counter-clockwise) it will be decremented.
				motor->currentPosition += motor->direction;
				
				// Keep track of relative current position
				motor->current_step++;
			}
		
			motor->ISR_step_count--; // Decrement step events count 
			
			if(motor->ISR_step_count < 1){
				// Segment is complete. Discard current segment and advance segment indexing.
				if(++motor->segment_buffer_tail == SEGMENT_BUFFER_SIZE){
					motor->segment_buffer_tail = 0;
					motor->segment_buffer_ovf = false;
				}
			}
		}
		
		
	} // end for loop
	
	// Test pin used to measure execution time
	//T_PIN_PORT &= ~(1 << T_PIN);
}