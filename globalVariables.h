/*
 * globalVariables.h
 *
 * Created: 14.11.2015 14:44:44
 *  Author: Lars
 
 Get and Set functions for global variables that need to be accessible from any function
 Definitions for states
 
 */ 

#ifndef GLOBAL_VARIABLES
#define GLOBAL_VARIABLES

enum MAINSTATE
{
	WAITING = 0,
	STOPPED,
	MOVING,
};

enum PUKSTATE
{
	PUK_NDEF = 0,
	TEAMMATE_HAS_PUK,
	OPPONENT_HAS_PUK,
	PUK_VISIBLE
};

enum MOVINGSTATE
{
	MOVING_NDEF = 0,
	LOOKING_FOR_PUK,
	MOVING_TO_GOAL,
	MOVING_TO_PUK,
	MOVING_TO_OPPONENT,
};

static void setMainState(int state);
static enum MAINSTATE getMainState();
static void setPukState(int state);
static enum PUKSTATE getPukState();
static void setMovingState(int state);
static enum MOVINGSTATE getMovingState();
static void setPosition(float *pos);
static float *getPosition();

#endif