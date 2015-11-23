#include <avr/io.h>
#include "m_general.h"
#include "globalVariables.h"

char buffer[10] = {0,0,0,0,0,0,0,0,0,0};

int message = 0;
int i = 0;
int run = 1;

void rf_comm(char* buffer) {
	while(run)
	{
		message = buffer[0];

		switch(message){

			case 0xA0: 	// Comm Test
				for(i=0; i<10; i++){
						set(PORTD,4); 	//Flash positioning LED 10 times
						clear(PORTD,4);
				}
				break;

			setMessage(1);

			case 0xA1:	//PLAY (TURN ON ROBOT)
			setMessage(2);
			break;

			case 0xA2: // GOAL R (STOP)
			setMessage(3);
			break;

			case 0xA3: //GOAL B (STOP)
			setMessage(4);
			break;

			case 0xA4: //PAUSE (TURN OFF ROBOT IN 3 SECONDS)
			setMessage(5);
			break;

			case 0xA6: //HALFTIME(STOP_)
			setMessage(6);
			break;

			case 0xA7: //GAME OVER (STOP)
			setMessage(7);
			break;

			case 0xA8: // Enemy positions
			setMessage(8);
			break;
		}

		run = 0;
	}
}
