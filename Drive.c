/*
 * Drive.c
 *
 * Created: 14.11.2015 01:06:19
 * Author: Lars Jebe
 * University of Pennsylvania
 * Robockey
 */

#include "Drive.h"
#include "globalVariables.h"
#include "m_general.h"
#include "m_bus.h"
#include "m_usb.h"
#include <math.h>


#define ACCURACY 30
#define DRIVE_PI 3.14159
#define DRIVE_KP 1
#define DRIVE_KD 10
#define MAX_DELTA_ANGLE 3.14159/6

static float DRIVE_POWER = 0;

static int goto_x = 0;
static int goto_y = 0;
static float turn_target = 0;
static int GOTO = 0;
static int TURN = 0;
static int SEARCH = 0;
static float delta_angle_prev = 0;
static float delta_angle = 0;

static float* position = 0;

void reset_drive() {
	GOTO = 0;
	TURN = 0;
	SEARCH = 0;
	set_power(1);
}

void drive_update()
{
	if(GOTO) {
		goTo(goto_x, goto_y);
	} else if(TURN) {
		turn(turn_target);
	} else if(SEARCH) {
		drive_search();
	}
}

void set_power(float power) {
	DRIVE_POWER = power;
}

void stop()
{
	GOTO = 0;
	setDriveState(DRIVE_NDEF);
	rightOFF();
	leftOFF();
}

float getPID(float target_angle) {
	if(!position[0]) {
		position = getPosition();
	}
	//get current angle
	float current_angle = position[2]; //already 0 -> 2pi

	//get delta for PID
	delta_angle = target_angle - current_angle;

	//handle edge case with zero rollover
	if(fabs(delta_angle) > DRIVE_PI) {
		if(delta_angle > 0) {
			delta_angle = delta_angle - 2 * DRIVE_PI;
		} else {
			delta_angle = delta_angle + 2 * DRIVE_PI;
		}
	}

	//drive PID
	float DRIVE_PID = (DRIVE_KP - fabs(delta_angle - delta_angle_prev) * DRIVE_KD);
	if(DRIVE_PID < 0) {
		DRIVE_PID = 0;
	}
	return DRIVE_PID;
}

void goTo(int x, int y) //goes to the specified position on the field (in cm)
{
	//save and update state
	goto_x = x;
	goto_y = y;
	setDriveState(GO_TO);

	reset_drive();
	GOTO = 1;

	position = getPosition();

	//get target angle
	float target_angle = atan2f(y - position[1], x - position[0]);

	//change target angle to 0 -> 2pi coords
	if(target_angle < 0) {
		target_angle = target_angle + 2 * DRIVE_PI;
	}

	float DRIVE_PID = getPID(target_angle); //update delta angle and get PID value
	float delta_power = fabs(delta_angle)* DRIVE_POWER * DRIVE_PID/ DRIVE_PI;
	delta_angle_prev = delta_angle;
	if(delta_power > 1) {
		delta_power = 1;
	}


	if(fabs(delta_angle) > MAX_DELTA_ANGLE) {
		if(delta_angle < 0) {
			leftON(delta_power, FORWARDS);
			rightON(delta_power, BACKWARDS);
		} else {
			leftON(delta_power, BACKWARDS);
			rightON(delta_power, FORWARDS);
		}
	} else {
		float right_power = DRIVE_POWER + delta_power * delta_angle / fabs(delta_angle);
		if(right_power > 1) {
			right_power = 1;
		}
		float left_power = DRIVE_POWER - delta_power * delta_angle / fabs(delta_angle);
		if(left_power > 1) {
			left_power = 1;
		}
		rightON(right_power, FORWARDS);
		leftON(left_power, FORWARDS);
	}

	m_green(ON);

	if(fabs(position[0] - goto_x) < 25 && fabs(position[1] - goto_y) < 25) {
		stop();
	}
}


void turn(float target_angle) //turns a certain angle in RAD
{
	turn_target = target_angle;
	reset_drive();
	TURN = 1;
	float DRIVE_PID = getPID(target_angle);
	float delta_power = fabs(delta_angle)* DRIVE_POWER * DRIVE_PID/ DRIVE_PI;
	if(delta_angle < 0) {
		leftON(delta_power, FORWARDS);
		rightON(delta_power, BACKWARDS);
	} else {
		leftON(delta_power, BACKWARDS);
		rightON(delta_power, FORWARDS);
	}
}

void goStraight(int distance, int direction, float velocity) //goes straight a certain distance (in cm), direction is either FORWARDS, BACKWARDS
{
	start_pwm1(256,velocity);
	start_pwm3(256,velocity);
	float current_position[3] = {0};
	float *current_position_p;
	current_position_p = getPosition();
	float target_position[2] = {current_position[1]+distance*cos(current_position[3]), current_position[2]+sin(current_position[3])};

	setMainState(MOVING);

	if (abs(target_position[1] - current_position[1])  < ACCURACY && abs(target_position[2] - current_position[2]) < ACCURACY )
	{
		stop1();
		stop3();
		setMainState(WAITING);
	}
}

void leftON(float power, int direction)
{
	if(power > 1) power = 1;

	start_pwm1(1024,power);

	if (direction != FORWARDS)
	{
		clear(PORTB, 0);
	}
	else
	{
		set(PORTB, 0);
	}
}

void leftOFF()
{
	stop1();
}

void rightON(float power, int direction)
{
	if(power > 1) power = 1;


	start_pwm3(1024,power);

	if (direction == FORWARDS)
	{
		clear(PORTB, 1);
	}
	else
	{
		set(PORTB, 1);
	}
}

void rightOFF()
{
	stop3();
}

void drive_search(){
	reset_drive();
	SEARCH = 1;
	set_power(0.5);

	//try to turn in a circle
	if(position[2] > 0 && position[2] < 2.9) {
		turn(3.14);
	} else if(position[2] > 2.9) {
		turn(-1.7);
	} else if(position[2] < -1.9 && position[2] > 0) {
		turn(0.5);
	}
}
