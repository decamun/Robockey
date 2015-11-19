
#include <avr/io.h>
#include "options.h"
#include "m_general.h"
#include "globalVariables.h"
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


//flags
int RF_READ = 0;
int TICK_HAPPENED = 0;

void report_error() {
	m_red(ON);
	if(USB_DEBUG && m_usb_isconnected()) {
		m_usb_tx_string("error!\n\r");
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

	//initialize RF
	m_rf_open(CHANNEL, ADDRESS, BUFFER_SIZE);

	//initialize USB
	if(USB_DEBUG || MATLAB_GRAPH) {
		m_usb_init();
	}

	//start timer0
	start0(TICK_FREQUENCY);
	interupt0(1);
}

void main()
{
	initialize();

	goTo(0, 0);
	float* pos_alt = NULL;
	while (1) {
		if(TICK_HAPPENED) {
			//handle new clock tick
			m_green(OFF);
			localize_update(); //update localization info
			drive_update(); //update drive state

			m_green(ON);


			//main loop things








			TICK_HAPPENED = 0;

			//usb_debug code
			if(USB_DEBUG && m_usb_isconnected()) {

				pos_alt = localize_location();
				int i = 0;

				m_usb_tx_string("\n\rPointer Address in main:");
	      m_usb_tx_int(pos_alt);

				m_usb_tx_string("\n\r");
				for(i = 0; i < 3; i++) {
					m_usb_tx_int((int)(pos_alt[i]*100));
					m_usb_tx_string(" | ");
				}
				m_usb_tx_string("\n\r");
			}
		}
		if(RF_READ) {
			//handle new RF info
			RF_READ = 0;
		}
	}
}

//m_rf flag setter
ISR(INT2_vect) {
	//m_rf_recieved a thing
	RF_READ = m_rf_read(buffer, BUFFER_SIZE);
}


//tick flag setter
ISR(TIMER0_OVF_vect) {
	TICK_HAPPENED = 1;
	if(USB_DEBUG && m_usb_isconnected) {
		m_usb_tx_string("TICK!\n\r");
	}
}
