/*
 * globalVariables.c
 *
 * Created: 14.11.2015 20:28:07
 *  Author: Lars
 */

#include "globalVariables.h"

enum MAINSTATE current_mainstate;
enum PUKSTATE current_pukstate;
enum MOVINGSTATE current_movingstate;

float *current_position;

void setMainState(int state)
{
	current_mainstate = state;
}

enum MAINSTATE getMainState()
{
	return current_mainstate;
}

void setPukState(int state)
{
	current_pukstate = state;
}

enum PUKSTATE getPukState()
{
	return current_pukstate;
}

void setMovingState(int state)
{
	current_movingstate = state;
}

enum MOVINGSTATE getMovingState()
{
	return current_movingstate;
}

void setPosition(float *pos)
{
	current_position = pos;
}

float *getPosition()
{
	return current_position;
}
