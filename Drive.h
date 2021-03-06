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

void reset_drive();
void goTo(int x, int y); //goes to the specified position on the field (in cm)
void drive_search();
float getPID(float target_angle);
void set_power(float power);
void turn(float target_angle); //turns a certain angle in RAD
void goStraight(int distance, int direction, float velocity); //goes straight a certain distance (in cm)
void leftON(float power, int direction);
void leftOFF();
void rightON(float power, int direction);
void rightOFF();
void drive_update();
void stop();
