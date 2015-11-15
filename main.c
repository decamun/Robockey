
#include <avr/io.h>
#include "m_general.h"
#include "m_usb.h"
#include "m_bus.h"
#include "m_rf.h"
#include "timer0.h"
#include "timer1.h"
#include "localize.h"
#include "globalVariables.h"
#include "ADC.h"
#include "Drive.h"
#include <math.h>
#include <stdint.h>


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

	while (1)
	{

	}
}
