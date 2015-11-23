#include <avr/io.h>
#include "m_general.h"
#include "m_rf.h"

#define CHANNEL 1
//#define RXADDRESS 0xC //For robot 1
//#define RXADDRESS 0xD //For robot 2
//#define RXADDRESS 0xE //For robot 3
#define PACKET_LENGTH 10
#define F_CPU 16000000

char buffer[PACKET_LENGTH] = {0,0,0,0,0,0,0,0,0,0};

int message = 0;

void rf_comm(int robotnum) {

	switch(robotnum){
		case 1:
		RXADDRESS = 0xC; // 12 For robot 1
		break;

		case 2:
		RXADDRESS = 0xD; // 13 For robot 2
		break;

		case 3
		RXADDRESS = 0xE; // 14 For robot 3
		break;

	}
	
	m_red(ON);
	while(!m_rf_open(CHANNEL, RXADDRESS, PACKET_LENGTH)){}	// Initializes mIMU
	m_red(OFF);

	while(TRUE)
	{

		if(flag == 1)
		{
		switch(message){

			case 0xA0: 	// Comm Test
				for(i=0; i<10; i++){
						set(PORTD,4); 	//Flash positioning LED 10 times
						m_wait(50);
						clear(PORTD,4)
						m_wait(50);
				}
				break;

			case 0xA1:	//PLAY (TURN ON ROBOT)

			break;

			case 0xA2: // GOAL R (STOP)

			break;

			case 0xA3: //GOAL B (STOP)

			break;

			case 0xA4: //PAUSE (TURN OFF ROBOT IN 3 SECONDS)

			break;

			case 0xA6: //HALFTIME(STOP_)

			break;

			case 0xA7: //GAME OVER (STOP)

			break;

			case 0xA8: // Enemy positions

			break;


		}

		flag = 0;
		}	
	}
}

///WIRELESS INTERRUPT 

ISR(INT2_vect){
	m_rf_read(buffer, PACKET_LENGTH);
	m_green(TOGGLE);

	message = buffer[0];
	


flag = 1;
}
