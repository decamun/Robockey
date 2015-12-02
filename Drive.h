/*
 * Drive.h
 *
 * Created: 14.11.2015 01:06:04
 * Author: Lars Jebe
 * University of Pennsylvania
 * Robockey
 */

#include "timer1.h" //output B6
#include "timer3.h" //output C6
#include "localize.h"
#include "globalVariables.h"
#include <math.h>

#define FORWARDS 0
#define BACKWARDS 1

typedef enum {STOP, PROGRESS, DONE} drive_state;
typedef enum {D_NONE, GOTO, SEARCH, TURN} drive_action;

/** Move forward while trying to achieve a target heading.
 * This function will change the base speed based on the distance of the target,
 * slowing as we get closer.
 * Note: This function will never run a motor in reverse. It will continue to
 * try to move forward while turning.
 */
void goTo(float* position, float target_angle, float target_dist);

/**
 * Resets PD controller set in the GoTo function
 */
void resetGoTo();

/*
 * Goes to the specified position on the field (in cm)
 */
void goTo(int x, int y); 
void drive_search();
void turn(float target_angle); //turns a certain angle in RAD
void goStraight(int distance, int direction, float velocity); //goes straight a certain distance (in cm)

float getAnglePID(float current_angle, float target_angle);
void set_power(float power);

void leftON(float power, int direction);
void leftON(float power);
void leftOFF();

void rightON(float power, int direction);
void rightON(float power);
void rightOFF();

void stop();

void get_drive_action();
void get_drive_state();


