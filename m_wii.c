bplist00�_WebMainResource�	
_WebResourceData_WebResourceMIMEType_WebResourceTextEncodingName_WebResourceFrameName^WebResourceURLO�<html class="gr__medesign_seas_upenn_edu"><head><style></style></head><body><pre style="word-wrap: break-word; white-space: pre-wrap;">// -----------------------------------------------------------------------------
// M2 Wiimote Pixart Sensor interface
// version: 1.0
// date: Oct 13, 2011
// author: J. Fiene
// -----------------------------------------------------------------------------

#include "m_general.h" // just for debugging
#include "m_wii.h"
#include "m_usb.h"

#define MWIITWIADDR		0x58
#define TWI_MAX_WAIT	1000
#define INTERPACKET		10

// private function prototypes
unsigned char start_write(unsigned char address);
unsigned char start_read(unsigned char address);
unsigned char send_byte(unsigned char byte);
unsigned char twi_wait(void);
void end(void);

char m_wii_open()
{	
	m_bus_init();

	// enter configuration mode
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x30)){return 0;}
	if(!send_byte(0x01)){return 0;}
	end();
	m_wait(INTERPACKET);
		
	// sensitivity setting, part 1
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x90)){return 0;} // p0
	end();

	m_wait(INTERPACKET);
	
	// sensitivity setting, part 2
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x07)){return 0;}
	if(!send_byte(0x00)){return 0;}
	if(!send_byte(0x41)){return 0;} // p1
	end();

	m_wait(INTERPACKET);
	
	// sensitivity setting, part 3
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x1A)){return 0;}
	if(!send_byte(0x40)){return 0;} // p2
	if(!send_byte(0x00)){return 0;} // p3
	end();

	m_wait(INTERPACKET);
	
	// not quite sure
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x33)){return 0;}
	if(!send_byte(0x03)){return 0;}
	end();

	m_wait(INTERPACKET);
	
	// not quite sure
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x30)){return 0;}
	if(!send_byte(0x08)){return 0;}
	end();
	
	m_wait(INTERPACKET);

	return 1;
}

char m_wii_read(unsigned int* blob_data)
{
	int i;
	unsigned char temp[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	// prepare for reading
	if(!start_write(MWIITWIADDR)){return 0;}
	if(!send_byte(0x36)){return 0;}
	end();

	_delay_us(25);
	
	if(!start_read(MWIITWIADDR)){return 0;}
	for(i=0;i&lt;15;i++)
	{
		TWCR = (1&lt;&lt;TWINT) | (1&lt;&lt;TWEA) | (1&lt;&lt;TWEN);	// clear the flag, enable ACKs, and wait for another byte
		while(!(TWCR &amp; (1&lt;&lt;TWINT))){}; // wait for an interrupt to signal that a new byte is available
		temp[i] = TWDR;
	}
	TWCR = (1&lt;&lt;TWINT) | (1&lt;&lt;TWEN);	// clear the flag, no ACK, and wait for another byte
	while(!(TWCR &amp; (1&lt;&lt;TWINT))){}; // wait for an interrupt to signal that a new byte is available
	temp[15] = TWDR;
	end();

	for(i=0;i&lt;10;i+=3) // for each blob:
	{	// i = 0, 3, 6, 9
		blob_data[i] = (unsigned int)temp[i+1] + ((int)(temp[i+3] &amp; 0b00110000) &lt;&lt;4);	// X
		blob_data[i+1] = (unsigned int)temp[i+2] + ((int)(temp[i+3] &amp; 0b11000000) &lt;&lt;2);	// Y
		blob_data[i+2] = temp[i+3] &amp; 0b00001111;										// size
	}
	return 1;
}


//
//
// PRIVATE FUNCTIONS BELOW
//
//


unsigned char start_write(unsigned char address)
{
	unsigned char status;
	// START
	TWCR = (1&lt;&lt;TWEN)|(1&lt;&lt;TWSTA)|(1&lt;&lt;TWINT);
	if(!twi_wait())
	{
		return 0;
	}
	
	// ADDRESS
	status = send_byte(address&lt;&lt;1);
	if(status== 0x20){ // ACK was not received - may not be connected/listening
		TWCR = (1&lt;&lt;TWINT)|(1&lt;&lt;TWEN)| (1&lt;&lt;TWSTO); // let go of the line (STOP)
		return 0;	// failure
	}	
	return 1;	// success
}	

unsigned char start_read(unsigned char address)
{
	unsigned char status;
	// START
	TWCR = (1&lt;&lt;TWEN)|(1&lt;&lt;TWSTA)|(1&lt;&lt;TWINT);
	if(!twi_wait())
	{
		return 0;
	}
	
	// ADDRESS
	status = send_byte(((address&lt;&lt;1) + 1));
	if(status== 0x48){ // ACK was not received - may not be connected/listening
		TWCR = (1&lt;&lt;TWINT)|(1&lt;&lt;TWEN)| (1&lt;&lt;TWSTO); // let go of the line (STOP)
		return 0;	// failure
	}	
	return 1;	// success
}	

unsigned char send_byte(unsigned char byte)
{
	TWDR = byte;					// load the byte
	TWCR = (1&lt;&lt;TWINT) | (1&lt;&lt;TWEN);	// send the byte
	if(twi_wait())					// timed out
	{
		return (TWSR &amp; 0xF8);		// return the status with prescaler bits masked out
	} else {
		return 0;					// comm failure
	}
}

unsigned char twi_wait(void)
{
	unsigned int wait=0;
	while((!(TWCR &amp; (1&lt;&lt;TWINT))) &amp;&amp; (wait++&lt;TWI_MAX_WAIT)){};	// wait for acknowledgement that they byte was sent
	if(wait==TWI_MAX_WAIT)
	{
		return 0;					// fail
	} else {
		return 1;					// success
	}
}

void end(void)
{
	// STOP
	TWCR = (1&lt;&lt;TWINT)|(1&lt;&lt;TWEN)| (1&lt;&lt;TWSTO);
}</pre><div class="grammarly-disable-indicator"></div></body><span class="gr__tooltip"><span class="gr__tooltip-content"></span><i class="gr__tooltip-logo"></i><span class="gr__triangle"></span></span></html>[text/x-csrcUUTF-8P_5http://medesign.seas.upenn.edu/uploads/Guides/m_wii.c    ( : P n � �lx~                           �