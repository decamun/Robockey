/*
 * Drive.h
 *
 * Created: 14.11.2015 01:06:04
 * Author: Lars Jebe
 * University of Pennsylvania
 * Robockey
 */ 

#include "timer1.h" //output B6

#define LEFT 0
#define RIGHT 1

void goTo(int x, int y); //goes to the specified position on the field (in cm)
void turn(int angle); //turns a certain number of DEGREES
void goStraight(int distance, int direction); //goes straight a certain distance (in cm)