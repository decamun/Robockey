
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
int TEAM_RED = 0;

int GO = 0;
int BLINK = 0;
int SEARCH_MODE = 0;
int ii = 0;

int KICK_TICKS = 0;
int LED_pin = 2;
int OFF_LED_Pin = 3;
void report_error(const char *err);
void initialize();
void kick();

typedef enum {SEARCHING = 0, ACQUIRE, GOTO_GOAL, PUCK_TURN, PAUSE, PLAY, GOTO_ZERO} robot_state;
static robot_state current_state = PAUSE;

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

void fullTestMotor() {
    float i = 0;
    for(i = 0; i < 1; i += 0.01) {
        setRight(i);
        setLeft(i);
        m_wait(200);
    }

    m_wait(1000);
    stop();
    m_wait(3000);

    setLeft(1);
    m_wait(2000);
    stop();
    m_wait(1000);
    setRight(1);
    m_wait(2000);
    stop();
    m_wait(1000);

    setLeft(-1);
    m_wait(2000);
    stop();
    m_wait(1000);
    setRight(-1);
    m_wait(2000);
    stop();
    m_wait(1000);
}

void testPuckRead() {
     update_puck_angle();
    m_usb_tx_long(get_puck_angle());
    m_usb_tx_string("\r\n");
}

void main()
{
    initialize();
    m_wait(1000);
    resetGoTo(); // Ensure that PD loops are set to 0

    //TODO: Change this back to PAUSE for real play
    current_state = PAUSE;

    while (1) {
        if(TICK_HAPPENED) {
           // Get the current position and orientation
            localize_update();
            update_puck_angle();

            m_usb_tx_string("State ");
            m_usb_tx_int(current_state);
            m_usb_tx_string("\r\n");
            m_usb_tx_string("Puck ");
            m_usb_tx_int((int) (100 * get_puck_angle()));
            m_usb_tx_string("\r\n");

            m_usb_tx_string("Pos (x, y, t): (");
            m_usb_tx_int((int) (getPosition()[0]));
            m_usb_tx_string(", ");
            m_usb_tx_int((int) (getPosition()[1]));
            m_usb_tx_string(", ");
            m_usb_tx_int((int) (getPosition()[2]));
            m_usb_tx_string(")\r\n");

            switch(current_state) {
                case PAUSE:
                    clear(PORTD, LED_pin); // TURN OFF positioning LED
                    setRight(0);
                    setLeft(0);
                    break;

                case PLAY:
                    set(PORTD, LED_pin);
                    current_state = SEARCHING;
                    break;

                case SEARCHING:
                    setRight(0.5f);
                    setLeft(-0.5f);

                    if(get_see_puck()) {
                        current_state = ACQUIRE;
                    }
                    break;

                case ACQUIRE:

                    goToHeadingVel(0.87f, get_puck_angle(), 0.0f); 

                    if (puck_middle() || ((int)(get_puck_distance()<3))) {
                        current_state = GOTO_GOAL;
                        resetGoTo();
                    }

                    if (!get_see_puck()) {
                        current_state = SEARCHING;
                        resetGoTo();
                    }

                    break;

                case GOTO_GOAL:
                    goToPosition(getPosition(), 0.77f, GOAL_X, GOAL_Y);

                    if (!get_see_puck()) {
                        current_state = SEARCHING;
                    } else if (!puck_middle()) {
                        current_state = ACQUIRE;
                    }

                    break;

                case PUCK_TURN:
                    if (puck_left()) {
                        setRight(0.8);
                    } else if (puck_left()) {
                        setLeft(0.8);
                    } else if (puck_middle()) {
                        current_state = GOTO_GOAL;
                    } else {
                        current_state = ACQUIRE;
                    }
                case GOTO_ZERO:
                    goToPosition(getPosition(), 0.7f, 0.0f, 0.0f);
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

}

void initialize() {
    m_clockdivide(0);
    //enable inputs for switches
    clear(DDRB, 4);
    set(PORTB, 4);
    clear(DDRB, 5);
    set(PORTB, 5);

    //Changing Output pin for different team
    if(puck_left() || puck_right()) {
      TEAM_RED = 1;
    } else {
      TEAM_RED = 0;
    }

    set(DDRB, 2);
    set(DDRB, 3);
    if(TEAM_RED)
    {
      set(PORTB, 3);
      clear(PORTB, 2);
    } else {
      set(PORTB, 2);
      clear(PORTB, 3);
    }

    clear(DDRD, 3);

    //Enabling Positioning LED Pins for output
    // set(DDRB, LED_pin);
    // set(PORTB, LED_pin);
    //
    // set(DDRB, OFF_LED_Pin);
    // clear(PORTB, OFF_LED_Pin);

    //driver board enable line
    //set(DDRB, 2); //enable output
    //set(PORTB, 2); //pull high

    //enable direction lines (v+?)
    set(DDRB, 0);
    set(DDRB, 1);

    //enable kicker ports
    set(DDRB, 7);
    clear(PORTB, 7);



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
    localize_init();
}


void report_error(const char *err) {
    m_red(ON);
    if(USB_DEBUG && m_usb_isconnected()) {
        m_usb_tx_string("ERROR: ");
        print_P(err); // same as m_usb_tx_string, getting around compile error
        m_usb_tx_string("\r\n");
    }
}

void kick()	{
    set(PORTB, 7);
    m_wait(200);
    clear(PORTB, 7);
    //KICK_TICKS = (int)(TICKS_PER_SECOND * 0.125);
}

void handleRfGamestate(uint8_t value) {
    if(value == 0xA0) { //Comm Test
        BLINK = 1;
        m_green(TOGGLE);
        m_wait(500);

        if(TEAM_RED) // Flashing light for comm test
    {
        for(ii = 0; ii<2; ii++){

          clear(PORTB,3);
          m_wait(200);
          set(PORTB,3);
          m_wait(200);
        }

    } else {  
        for(ii = 0; ii<2; ii++){

          clear(PORTB,2);
          m_wait(200);
          set(PORTB,2);
          m_wait(200);
        }
    }

    } else if (value == 0xA1) { // Play
        current_state = PLAY;

    } else if(value == 0xA2) { // Goal R
        current_state = PAUSE;

    } else if(value == 0xA3) { // Goal B
        current_state = PAUSE;

    } else if(value == 0xA4) { // Pause
        current_state = PAUSE;

    } else if(value == 0xA5) { // detangle
        current_state = PAUSE;

    } else if(value == 0xA6) { // Halftime
        current_state = PAUSE;

        if(TEAM_RED){
            TEAM_RED = 0; 
            clear(PORTB,3); // Change LED color at halftime
            set(PORTB,2);
        }
        else{
            TEAM_RED = 1; 
            clear(PORTB,2);
            set(PORTB,3);
        }



    } else if(value == 0xA7) { // Game Over
        current_state = PAUSE;
    }
}

//m_rf flag setter
ISR(INT2_vect) {
    RF_READ = 1;
    m_rf_read(buffer, BUFFER_SIZE);

    m_usb_tx_string("RF: ");
    m_usb_tx_hex(buffer[0]);
    m_usb_tx_hex("\r\n");

    handleRfGamestate((uint8_t) buffer[0]);
}


//tick flag setter
ISR(TIMER0_OVF_vect) {
    if(TICK_HAPPENED) {
        m_red(ON);
    } else {
        m_red(OFF);
        TICK_HAPPENED = 1;
        if(USB_DEBUG && m_usb_isconnected) {
            //m_usb_tx_string("TICK!\n\r");
        }
    }
}
