/*
 * globalVariables.h
 *
 * Created: 14.11.2015 14:44:44
 *  Author: Lars
 
 Get and Set functions for global variables that need to be accessible from any function
 
 */ 

volatile enum STATE
{
	state0 = 0,
	state1, 
	state2
} current_state;
	
float current_position[3]; 


void setState(int state)
{
	current_state = state;
}

enum STATE getState()
{
	return current_state;
}

void setPosition(float *pos)
{
	current_position = pos;
}

float* getPosition()
{
	return current_position;
}
