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

  void setMainState(int state);
  enum MAINSTATE getMainState();
  void setPukState(int state);
  enum PUKSTATE getPukState();
  void setMovingState(int state);
  enum MOVINGSTATE getMovingState();
  void setPosition(float *pos);
  float *getPosition();

#endif
