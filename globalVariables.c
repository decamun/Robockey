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
enum DRIVESTATE current_drivestate;
enum MESSAGE current_message;

float *current_position;

void setDriveState(int state)
{
	current_drivestate = state;
}

enum DRIVESTATE getDriveState()
{
	return current_drivestate;
}

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

void setMessage(int state)
{
	current_message = state;
}

enum MESSAGE getMessage()
{
	return current_message;
}

void setPosition(float *pos)
{
	current_position = pos; //[x, y, angle]
}

float *getPosition()
{
	return current_position;
}
