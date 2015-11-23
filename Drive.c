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

static int goto_x = 0;
static int goto_y = 0;
static float* position = 0;

void drive_update()
{
	//if(getDriveState == GO_TO) {
	goTo(goto_x, goto_y);
		//leftON(124, 0);
		//rightON(124, 0);
	//}
}

void stop()
{
	setDriveState(DRIVE_NDEF);
	rightOFF();
	leftOFF();
}

void goTo(int x, int y) //goes to the specified position on the field (in cm)
{
	//save and update state
	goto_x = x;
	goto_y = y;
	setDriveState(GO_TO);
	position = getPosition();
	m_usb_tx_int((int)(position[2] * 100));
	m_usb_tx_string("!!!!!!!!!!!!\n\r");

	//get target angle
	float target_angle = atan2f(y - position[1], x - position[0]);

	//change target angle to 0 -> 2pi coords
	if(target_angle < 0) {
		target_angle = target_angle + 2 * DRIVE_PI;
	}

	//get current angle
	float current_angle = position[2]; //already 0 -> 2pi

	//get delta for PID
	float delta_angle = target_angle - current_angle;

	//handle edge case with zero rollover
	if(fabs(delta_angle) > DRIVE_PI) {
		if(delta_angle > 0) {
			delta_angle = delta_angle - 2 * DRIVE_PI;
		} else {
			delta_angle = delta_angle + 2 * DRIVE_PI;
		}
	}


	if(delta_angle > 0) {
		m_red(OFF);
		//cw
		rightON(0.5, FORWARDS);
		leftON(0.3, FORWARDS);
		m_green(ON);
	} else if(delta_angle < 0) {
		m_red(ON);
		//ccw
		rightON(0.3, FORWARDS);
		leftON(0.5, FORWARDS);
	} else {
		stop();
	}
}


void turn(int angle, float velocity) //turns a certain angle in RAD
{

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
	start_pwm1(1024,power);

	if (direction != FORWARDS)
	{
		set(PORTB, 0);
	}
	else
	{
		clear(PORTB, 0);
	}
}

void leftOFF()
{
	stop1();
}

void rightON(float power, int direction)
{


	start_pwm3(1024,power);

	if (direction == FORWARDS)
	{
		set(PORTB, 1);
	}
	else
	{
		clear(PORTB, 1);
	}
}

void rightOFF()
{
	stop3();
}
