
#include <avr/io.h>
#include "options.h"
#include "m_general.h"
#include "globalVariables.h"
#include "Comm_Protocol.h"
#include "m_usb.h"
#include "m_bus.h"
#include "m_rf.h"
#include "timer0.h"
#include "timer1.h"
#include "localize.h"
#include "ADC.h"
#include "Drive.h"
#include <math.h>
#include <stdint.h>

char buffer[BUFFER_SIZE];

float* position = NULL;


//flags
int RF_READ = 0;
int TICK_HAPPENED = 0;
int GO = 0;
int BLINK = 0;

void report_error();

void initialize();

void main()
{
	initialize();
	//m_wait(5000);
	int i;
	for (i = 0; i < 50; i++) {
		localize_update();
		position = getPosition();
	}







	float* pos_alt = NULL;
	while (1) {
		if(TICK_HAPPENED) {
			//handle new clock tick
			localize_update(); //update localization info
			if(GO) {
				drive_update(); //update drive state
			}
			if(BLINK) {
				BLINK =0;
				int i;
				for(i = 0; i <20; i++) {
					m_green(TOGGLE);
					m_wait(100);
				}
			}
			//main loop things







			TICK_HAPPENED = 0;
		}


		if(RF_READ) {
			//handle new RF info
			RF_READ = 0;
			rf_comm(buffer);
		}
	}
}

void initialize() {
	m_clockdivide(0);

	//driver board enable line
	set(DDRB, 2); //enable output
	set(PORTB, 2); //pull high

	//enable direction lines
	set(DDRB, 0);
	set(DDRB, 1);

	set_power(INITIAL_POWER);

	//initialize RF
	m_rf_open(CHANNEL, ADDRESS, BUFFER_SIZE);

	//initialize USB
	if(USB_DEBUG || MATLAB_GRAPH || FORCE_USB) {
		m_usb_init();
	}

	//start timer0
	start0(TICK_FREQUENCY);
	interupt0(1);
}


void report_error() {
	m_red(ON);
	if(USB_DEBUG && m_usb_isconnected()) {
		m_usb_tx_string("error!\n\r");
	}
}

//m_rf flag setter
ISR(INT2_vect) {
	//m_rf_recieved a thing
	RF_READ = 1;
	m_rf_read(buffer, BUFFER_SIZE);
	//m_red(TOGGLE);
	if((uint8_t)buffer[0] == 0xA0) { //Comm Test
		BLINK = 1;
	} else if((uint8_t)buffer[0] == 0xA1) { // Play
		//m_green(TOGGLE);
		if (position[0] > 0)
		{
			goTo(-300, 0);
		}
		else {
			goTo(300, 0);
		}
		GO = 1;
	} else if((uint8_t)buffer[0] == 0xA2) { // Goal R
		GO = 0;
		stop();
	} else if((uint8_t)buffer[0] == 0xA3) { // Goal B
		GO = 0;
		stop();
	} else if((uint8_t)buffer[0] == 0xA4) { // Pause
		GO = 0;
		stop();
	}	else if((uint8_t)buffer[0] == 0xA5) { // detangle
		GO = 0;
		stop();
	} else if((uint8_t)buffer[0] == 0xA6) { // Halftime
		GO = 0;
		stop();
	} else if((uint8_t)buffer[0] == 0xA7) { // Game Over
		GO = 0;
		stop();
	}
}


//tick flag setter
ISR(TIMER0_OVF_vect) {
	TICK_HAPPENED = 1;
	if(USB_DEBUG && m_usb_isconnected) {
		m_usb_tx_string("TICK!\n\r");
	}
}
