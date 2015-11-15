
#include <avr/io.h>
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

char buffer[3] =  {3,3,3};

void main()
{
	m_clockdivide(0);

	//driver board enable line
	set(DDRB, 2); //enable output
	set(PORTB, 2); //pull high

	//enable direction lines
	set(DDRB, 1);
	set(DDRB, 2);

	leftON(0.3, FORWARDS);

	rightON(0.3, FORWARDS);


	//test comms
	m_rf_open(22, 0x000, 3);

	while (1) {
		m_rf_send(0x14, buffer, 3);
	}
}
