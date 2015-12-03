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
#define DRIVE_KP 3
#define DRIVE_KD 500
#define MAX_DELTA_ANGLE (3.14159f/6)
#define FIXED_DT 0.1f

#define GOTO_KP 1.0f
#define GOTO_KD 0.1f
#define MAX_DIST 100.0f
#define GOTO_POWER_KP 1.0f
#define GOTO_POWER_KD 0.1f

static float DRIVE_POWER = 0.0f;

// Variables for GOTO
static int goto_x = 0;
static int goto_y = 0;

// Variables for TURN
static float turn_target = 0;

static float delta_angle_prev = 0;
static float delta_angle = 0;

static float* position = 0;

static drive_state curr_drive_state = STOP;
static drive_action curr_action = D_NONE;

drive_action get_drive_action() {
    return curr_action;
}

drive_state get_drive_state() {
    return curr_drive_state;
}

void set_power(float power) {
	DRIVE_POWER = power;
}

void stop()
{
    curr_action = D_NONE;
    curr_drive_state = STOP;
	rightOFF();
	leftOFF();
}

static float goto_prev_error = 0.0f;
static float goto_prev_time = 0.0f;
static float goto_prev_distance = 0.0f;

void resetGoTo() {
    goto_prev_error = 0.0f;
    goto_prev_time = 0.0f;
    goto_prev_distance = 0.0f;
}

/**
 * PD loop on two angle measurements
 */
float getAnglePID2(float current_angle, float target_angle) {
    float delta_angle = target_angle - current_angle;

    //handle edge case with zero rollover
	if(fabs(delta_angle) > DRIVE_PI) {
		if(delta_angle > 0.0f) {
			delta_angle = delta_angle - 2.0f * DRIVE_PI;
		} else {
			delta_angle = delta_angle + 2.0f * DRIVE_PI;
		}
	}

    // Scale error to be between 0 and 1
    float error = delta_angle / DRIVE_PI;

    float res = GOTO_KP * error + GOTO_KD * ((error - goto_prev_error) / ((float)FIXED_DT));
    goto_prev_error = error;

    m_usb_tx_string("Error, Raw: ");
    m_usb_tx_int((int)(100* error));
    m_usb_tx_string("\t");
    m_usb_tx_int((int)((100 * (delta_angle))));

    m_usb_tx_string("\r\n");

    m_usb_tx_string("PD: ");
    m_usb_tx_int((int)(100 * res));
    m_usb_tx_string("\r\n");

    return res;
}


void goToPosition(float* position, float base_power, float target_x, float target_y) {
    float target_angle = atan2f(target_y - position[1], target_x - position[0]);
    float res = getAnglePID2(0, target_angle);

    float left_power;
    float right_power;

    if(res > 0) {
        left_power = res + base_power;
        right_power = base_power;
    } else {
        left_power = base_power;
        right_power = res + base_power;
    }

    setRight(left_power);
    setLeft(right_power);
}

void goToHeading(float* position, float target_angle, float target_dist) {
    float current_angle = 0;
    float res = getAnglePID2(current_angle, target_angle);

    // Scale distance to be between 0 and 1, if distance is above 1 then clamp
    float dist_error = (target_dist > MAX_DIST) ? 1 :  target_dist / MAX_DIST;
    float base_power = GOTO_POWER_KP * dist_error + GOTO_POWER_KD * (dist_error - goto_prev_distance) / FIXED_DT;
    goto_prev_distance = target_dist;

    // Always move forward by a factor of base_power
    // Bias a turning direction based on the PD for angle

    //TODO: Verify that this direction is correct

    float left_power;
    float right_power;

    if(res > 0) {
        left_power = res + base_power;
        right_power = base_power;
    } else {
        left_power = base_power;
        right_power = -res + base_power;
    }

    setRight(left_power);
    setLeft(right_power);
}

float getAnglePID(float current_angle, float target_angle) {
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
	} else if(DRIVE_PID > 1) {
		DRIVE_PID = 1;
	}
	delta_angle_prev = delta_angle;
	return DRIVE_PID;
}

void goTo(int x, int y) //goes to the specified position on the field (in cm)
{
	//save and update state
	goto_x = x;
	goto_y = y;

	position = getPosition();

	//get target angle
	float target_angle = atan2f(y - position[1], x - position[0]);

	//change target angle to 0 -> 2pi coords
	if(target_angle < 0) {
		target_angle = target_angle + 2 * DRIVE_PI;
	}

	float DRIVE_PID = getAnglePID(position[2], target_angle); //update delta angle and get PID value
	float delta_power = fabs(delta_angle)* DRIVE_POWER * DRIVE_PID/ DRIVE_PI;
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

	float DRIVE_PID = getAnglePID(getPosition()[2], target_angle);
	float delta_power = fabs(delta_angle)* DRIVE_POWER * DRIVE_PID/ DRIVE_PI;
	if(delta_power > 1) {
		delta_power = 1;
	}
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


	if (abs(target_position[1] - current_position[1])  < ACCURACY && abs(target_position[2] - current_position[2]) < ACCURACY )
	{
		stop1();
		stop3();
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

void setLeft(float power) {
    int direction = FORWARDS;
    if(power < 0.0f) {
        power = -power;
        direction = BACKWARDS;
    }

    leftON(power, direction);

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

void setRight(float power) {
    int direction = FORWARDS;
    if(power < 0.0f) {
        power = -power;
        direction = BACKWARDS;
    }

    rightON(power, direction);
}


void rightOFF()
{
	stop3();
}

void drive_search(){
	position = getPosition();
	float power  = getAnglePID(position[2], position[2] + 0.5);
	rightON(power, FORWARDS);
	leftON(power, BACKWARDS);
}
