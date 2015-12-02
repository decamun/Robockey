
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
#include "Puck_Find.h"

char buffer[BUFFER_SIZE];


//flags
int RF_READ = 0;
int TICK_HAPPENED = 0;

int GO = 0;
int BLINK = 0;
int SEARCH_MODE = 0;

int KICK_TICKS = 0;
int LED_pin = 5;
void report_error();
void initialize();
void kick();

typedef enum {SEARCHING = 0, ACQUIRE, GOTO_GOAL} robot_state;

void testMotors() {
    setRight(0.5);
    setLeft(0.5);
    m_wait(3000);
    stop();
    m_wait(3000);
    setRight(-0.5);
    setLeft(-0.5);
    m_wait(3000);
}

void main()
{
    initialize();
    m_wait(1000);

    robot_state current_state = SEARCHING;
    while(1) { testMotors(); }

    while (1) {
        if(TICK_HAPPENED) {
            // Get the current position and orientation
            localize_update();
            update_puck_angle();

            switch(current_state) {
                case SEARCHING: 
                    setRight(0.2);
                    setLeft(-0.2);

                    if(get_see_puck()) {
                        current_state = ACQUIRE;
                    }
                    break;

                case ACQUIRE:
                    goToHeading(getPosition(), get_puck_angle(), 10);

                    if (puck_middle()) {
                        current_state = GOTO_GOAL;
                        resetGoTo();
                    }

                    if (!get_see_puck()) {
                        current_state = SEARCHING; 
                        resetGoTo();
                    }

                    break;

                case GOTO_GOAL:
                    goTo(GOAL_X, GOAL_Y);
                    if (!get_see_puck()) {
                        current_state = SEARCHING;
                    } else if (!puck_middle()) {
                        current_state = ACQUIRE;
                    }

                    break;
            }
            // We're done until the next clock update 
            TICK_HAPPENED = 0;
        }

        if(RF_READ) {
            //handle new RF info
            RF_READ = 0;
            rf_comm(buffer);
        }

    }

    //	SEARCH_MODE = 1;
    //	while (1) {
    //		//remove this!!!!
    //		if(countdown > -1) {
    //			countdown = countdown -1;
    //		}
    //		if(puck_left() || puck_right() || puck_middle()) {
    //			m_green(OFF);
    //		} else {
    //			m_green(ON);
    //		}
    //
    //		if(TICK_HAPPENED) {
    //			//handle new clock tick
    //			localize_update(); //update localization info
    //			position = getPosition();
    //			//m_green(OFF);
    //
    //			//handle driving
    //			if(GO) {
    //			//	drive_update(); //update drive state
    //			} else {
    //			//	stop();
    //			}
    //
    //			//handle comm test
    //			if(BLINK) {
    //				BLINK =0;
    //					set(PORTD,LED_pin);
    //					m_wait(100);
    //					clear(PORTD,LED_pin);
    //
    //				int i;
    //				for(i = 0; i <10; i++) {
    //					m_green(TOGGLE);
    //					m_wait(100);
    //				}
    //
    //			}
    //
    //
    //			//handle kicking
    //			if(KICK_TICKS > 0) {
    //				KICK_TICKS = KICK_TICKS - 1;
    //				//kick
    //				set(PORTB, 7);
    //			} else {
    //				//don't kick
    //				clear(PORTB, 7);
    //			}
    //
    //			//handle searching
    //			/*if(SEARCH_MODE) {
    //				update_puck_angle();
    //				if(get_see_puck()) {
    //					//sees the puck
    //					//set_power(1);
    //					//turn(position[2] + get_puck_angle());
    //
    //					float delta_angle = get_puck_angle();
    //					if(fabs(delta_angle) < 3.14159/6) {
    //						leftON(1, FORWARDS);
    //						rightON(1, FORWARDS);
    //						if(countdown < 0 && !shot && (position[0] > 750 || position[0] < -750)) {
    //							//kick();
    //							shot = 1;
    //						}
    //					} else {
    //						set_power(1);
    //						turn(position[2] + get_puck_angle());
    //					}
    //					//leftON(0.3, FORWARDS);
    //					//rightON(0.3, FORWARDS);
    //				} else {
    //					//doesn't see the puck: go search for it
    //					//drive_search();
    //					leftON(1, FORWARDS);
    //					rightON(1, BACKWARDS);
    //
    //				}
    //			}*/
    //
    //			rightON(1, BACKWARDS);
    //
    //			//main loop things
    //
    //
    //
    //
    //
    //
    //			TICK_HAPPENED = 0;
    //		}
    //
    //
    //		if(RF_READ) {
    //			//handle new RF info
    //			RF_READ = 0;
    //			rf_comm(buffer);
    //		}
    //	}
}

void initialize() {
    m_clockdivide(0);

    //Changing Output pin for different team
    if(RED)
    {
        LED_pin = 6;

    }

    clear(DDRD, 3);

    //Enabling Positioning LED Pins for output
    //set(DDRD, 5);
    //set(DDRD, 6);

    //driver board enable line
    set(DDRB, 2); //enable output
    set(PORTB, 2); //pull high

    //enable direction lines
    set(DDRB, 0);
    set(DDRB, 1);

    //enable kicker ports
    set(DDRB, 7);

    //enable inputs for switches
    clear(DDRB, 4);
    set(PORTB, 4);
    clear(DDRB, 5);
    set(PORTB, 5);

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
    ADC_init();
}


void report_error() {
    m_red(ON);
    if(USB_DEBUG && m_usb_isconnected()) {
        m_usb_tx_string("error!\n\r");
    }
}

void kick()	{
    KICK_TICKS = (int)(TICKS_PER_SECOND * 0.125);
}

//m_rf flag setter
ISR(INT2_vect) {
    float* position = getPosition();
    //m_rf_recieved a thing
    RF_READ = 1;
    m_rf_read(buffer, BUFFER_SIZE);
    //m_red(TOGGLE);
    if((uint8_t)buffer[0] == 0xA0) { //Comm Test
        BLINK = 1;
    } else if((uint8_t)buffer[0] == 0xA1) { // Play
        //m_green(TOGGLE);
        set(PORTD,LED_pin); // Turn and Keep on Positioning LED

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
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    } else if((uint8_t)buffer[0] == 0xA3) { // Goal B
        GO = 0;
        stop();
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    } else if((uint8_t)buffer[0] == 0xA4) { // Pause
        GO = 0;
        stop();
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    }	else if((uint8_t)buffer[0] == 0xA5) { // detangle
        GO = 0;
        stop();
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    } else if((uint8_t)buffer[0] == 0xA6) { // Halftime
        GO = 0;
        stop();
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    } else if((uint8_t)buffer[0] == 0xA7) { // Game Over
        GO = 0;
        stop();
        clear(PORTD, LED_pin); // TURN OFF positioning LED
    }
}


//tick flag setter
ISR(TIMER0_OVF_vect) {
    if(TICK_HAPPENED) {
        m_red(ON);
    } else {
        m_red(OFF);
        TICK_HAPPENED = 1;
        if(USB_DEBUG && m_usb_isconnected) {
            m_usb_tx_string("TICK!\n\r");
        }
    }

}
