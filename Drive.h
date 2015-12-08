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
void goToHeading(float* position, float target_angle, float target_dist);

void trackHeading(float target_angle, float current_angle);


/**
 * Go to a heading at a constant base_power velocity 
 * Use 0.87f for aggressive maneuvering
 */
void goToHeadingVel(float base_power, float target_angle, float current_angle, float KP, float KD);


/**
 * Use the same turn/move forward behavior as goToHeading, except that the
 * target is an x,y position.
 */
void goToPosition(float *position, float base_power, float K, float target_x, float target_y);

/**
 * Resets PD controller set in the GoTo function
 */
void resetGoTo();

/*
 * Goes to the specified position on the field (in cm)
 */

float getAnglePID2(float curr, float target, float KP, float KD);

float headingToTarget(float *position, float target_x, float target_y);

void goTo(int x, int y); 
void drive_search();
void turn(float target_angle); //turns a certain angle in RAD
void goStraight(int distance, int direction, float velocity); //goes straight a certain distance (in cm)

float getAnglePID(float current_angle, float target_angle);
void set_power(float power);

void leftON(float power, int direction);
void setLeft(float power);
void leftOFF();

void rightON(float power, int direction);
void setRight(float power);
void rightOFF();

void stop();
void spin();

drive_action get_drive_action();
drive_state get_drive_state();


