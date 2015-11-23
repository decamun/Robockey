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
	goto_x = x;
	goto_y = y;
	setDriveState(GO_TO);
	position = getPosition();
	m_usb_tx_int((int)(position[2] * 100));
	m_usb_tx_string("!!!!!!!!!!!!\n\r");


	float target_angle = atan2f(y - position[1], x - position[0]);
	float current_angle = position[2];
	while (current_angle > 3.14159) {
		current_angle = current_angle - 2 * 3.14159;
	}
	float delta_angle = target_angle - current_angle;
	if(delta_angle > 3.14159) {
		delta_angle = -2 * 3.14159 + delta_angle;
	}
	if(delta_angle < -3.14159) {
		delta_angle = 2 * 3.14159 + delta_angle;
	}

	if(delta_angle > 0) {
		m_red(OFF);
		//cw
		rightON(0.5, FORWARDS);
		leftON(0.5, BACKWARDS);
		m_green(ON);
	} else if(delta_angle < 0) {
		m_red(ON);
		//ccw
		rightON(0.5, BACKWARDS);
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
