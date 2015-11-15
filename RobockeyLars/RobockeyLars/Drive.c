/*
 * Drive.c
 *
 * Created: 14.11.2015 01:06:19
 * Author: Lars Jebe
 * University of Pennsylvania
 * Robockey
 */ 

#include "Drive.h"
#include "m_general.h"

#define ACCURACY 30


void goTo(int x, int y) //goes to the specified position on the field (in cm)
{
	
}


void turn(int angle, float velocity) //turns a certain angle in RAD
{
	
}


void goStraight(int distance, int direction, float velocity) //goes straight a certain distance (in cm), direction is either FORWARDS, BACKWARDS
{
	start_pwm1(256,velocity); 
	start_pwm3(256, velocity);
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
	set(DDRB, 0);
	if (direction == FORWARDS)
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
	clear(DDRB, 0);
}

void reightON(float power, int direction)
{
	set(DDRB, 1);
	
	if (direction == FORWARDS)
	{
		set(PORTB, 0);
	}
	else
	{
		clear(PORTB, 0);
	}
}

void rightOFF()
{
	clear(DDRB, 1);
}
