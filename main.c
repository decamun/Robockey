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
char* ROBOT_ADDRESSES[3];

int GO = 0;
int BLINK = 0;
int SEARCH_MODE = 0;
int ii = 0;

int KICK_TICKS = 0;
int LED_pin = 2;
int OFF_LED_Pin = 3;

int indicators[3] = {0,1,2};

void report_error(const char *err);
void initialize();
void kick();

typedef enum {SEARCHING = 0, ACQUIRE, GOTO_GOAL, PUCK_TURN, PAUSE, PLAY, GOTO_ZERO,
              GOTO_GUARD, SEARCH_LEFT, SEARCH_RIGHT, TRACK, FACE_GUARD} robot_state;
static robot_state current_state = PAUSE;

void avoid_wall() {
    if(localize_heading_for_wall()) {
        setRight(-0.5f);
        setLeft(-0.5f);
        m_wait(200);
    }
}

void test_kicker() {
  kick();
  m_wait(3000);
}

void test_rf() {
  char out_buffer[10] = {0xA9,0x69,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  int i;
  for(i = 1; i <= 3; i++) {
    if(i != ROBOT_NUMBER) {
      m_usb_tx_string("Sending to Addess: ");
      m_usb_tx_int((uint8_t)ROBOT_ADDRESSES[i-1]);
      m_rf_send(ROBOT_ADDRESSES[i-1], out_buffer, 10);
      m_usb_tx_string("\n\r");
    }
  }
  m_usb_tx_string("Sending from Address: ");
  m_usb_tx_int((uint8_t)ROBOT_ADDRESSES[ROBOT_NUMBER-1]);
  m_usb_tx_string("\n\r");
  m_green(TOGGLE);
  m_wait(1000);
}

void set_indicators(robot_state _01, robot_state _11){
    indicators[0] = _01;
    indicators[1] = _11;

    set(DDRD, 5);
    set(DDRD, 7);
}

void update_indicators(){
  int i = 0;
    for (i = 0; i < 3; i++){
      if (indicators[0] == current_state){
          clear(PORTD, 5);
          set(PORTD, 7);
      }
      else if (indicators[1] == current_state){
          set(PORTD, 5);
          set(PORTD, 7);
      }
      else {
        clear(PORTD, 5);
        clear(PORTD, 7);
      }
    }
}

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

void goalie() {

    float angle = getPosition()[2];
    float GUARD_X = getPosition()[0];
    float GUARD_Y = getPosition()[1];

    switch(current_state) {
        case PAUSE:
            clear(PORTD, LED_pin); // TURN OFF positioning LED
            stop();
            break;
        case PLAY:
            current_state = GOTO_GUARD;
        case GOTO_GUARD:
            goToPosition(getPosition(), 0.5f, 0.5f, GUARD_X, GUARD_Y);
            float eps = 30.0f;
            if (fabs(getPosition()[0] - GUARD_X) < eps && fabs(getPosition()[1] - GUARD_Y) < eps) {
                current_state = FACE_GUARD;
            }
            break;
        case FACE_GUARD:
            setLeft(0.6f);
            setRight(-0.6f);
            if (fabs(getPosition()[2]) < 0.1f) {
                current_state = SEARCH_LEFT ;
            }
            break;
        case SEARCH_LEFT:
            setRight(-0.5f);
            setLeft(0.5f);
                if(angle > (DRIVE_PI / 2.0f) && angle < DRIVE_PI) {
                    current_state = SEARCH_RIGHT;
                }

            if (get_see_puck()) {
                current_state = TRACK;
            }

      //      if(fabs(getPosition()[0] - GUARD_X) > eps && fabs(getPosition()[1] - GUARD_Y) > eps);
      //        {
    //            current_state = GOTO_GUARD;
    //        }
            break;
        case SEARCH_RIGHT:
            setRight(0.5f);
            setLeft(-0.5f);
                if(angle >  DRIVE_PI && angle < 3.0f * DRIVE_PI / 2.0f) {
                    current_state = SEARCH_LEFT;
                }
            if (get_see_puck()) {
                current_state = TRACK;
            }

            break;
        case TRACK:
            trackHeading(-get_puck_angle(), 0.0f);

            if (!get_see_puck()) {
                current_state = SEARCH_LEFT;
            }
          //if (get_puck_position()[0]<-200) {
            if (get_puck_distance() < 40) {
                current_state = ACQUIRE;
            }
            break;
        case ACQUIRE:
            goToHeadingVel(0.6f, -(1.2f * get_puck_angle()), 0.0f, 3.0f, 2.0f);

            if (!get_see_puck()) {
                current_state = GOTO_GUARD;
            }

            if (puck_middle()) {
                current_state = GOTO_GOAL;
                resetGoTo();
            }

        case GOTO_GOAL:

            break;
    }
}

void forward() {
    switch(current_state) {
        case PAUSE:
            clear(PORTD, LED_pin); // TURN OFF positioning LED
            stop();
            break;

        case PLAY:
            set(PORTD, LED_pin);
            current_state = SEARCHING;
            break;

        case SEARCHING:
            spin();
            avoid_wall();
            if(get_see_puck())  {
                current_state = ACQUIRE;
            }
            break;

        case ACQUIRE:


            if(get_puck_distance() < 20.0f) {
                goToHeadingVel(0.6f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);
            } else {
                goToHeadingVel(0.77f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);

            }
            //} else {
            //    goToHeadingVel(0.7f, -get_puck_angle(), 0.0f);
            //}

            //if (fabs(headingToTarget(getPosition(), -GOAL_X, GOAL_Y)) < DRIVE_PI / 6.0f && getPosition()[0] > 50) {
            //    goToHeadingVel(0.5f, -get_puck_angle(), 0.0f);
            //} else {
            //}
            //bool puck_in_front = ((int)(get_puck_distance() < 3) && fabs(get_puck_angle()) < DRIVE_PI / 6);
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
            goToPosition(getPosition(), 0.3f, 0.9f, GOAL_X, GOAL_Y);
            //int eps = 50;

            //if(fabs(GOAL_X - getPosition()[0]) < 50 && fabs(GOAL_Y - getPosition()[1] < 100)) {
            //    float target_angle = atan2(GOAL_Y - getPosition()[1], GOAL_X - getPosition()[0]);
            //    float curr_angle = getPosition()[2];
            //    curr_angle = (curr_angle > DRIVE_PI) ? -(2 * DRIVE_PI - curr_angle) : curr_angle;
            //    float delta_angle = fabs(target_angle - curr_angle);
            //    if (delta_angle < DRIVE_PI / 6) {
            //        kick();
            //    }
            //}
            //


            if(!(puck_middle())) {
                current_state = ACQUIRE;
                resetGoTo();
            } else if (!get_see_puck()) {
                current_state = SEARCHING;
                resetGoTo();
            }

            break;

        case PUCK_TURN:
            if (puck_left()) {
                setRight(0.8f);
            } else if (puck_left()) {
                setLeft(0.8f);
            } else if (puck_middle()) {
                current_state = GOTO_GOAL;
            } else {
                current_state = ACQUIRE;
            }
            break;

        case GOTO_ZERO:
            goToPosition(getPosition(), 0.7f, 0.8f, GOAL_X, GOAL_Y);
            avoid_wall();
            break;
    }
}

void main()
{
    initialize();
    m_wait(1000);
    localize_update();


    resetGoTo(); // Ensure that PD loops are set to 0

    //TODO: Change this back to PAUSE for real play
    current_state = PLAY;
    while(1) {
      test_rf();
    }
    while (1) {
        if(TICK_HAPPENED) {
            // Get the current position and orientation
            localize_update();
            update_puck_angle();

            m_usb_tx_string("State ");
            m_usb_tx_int(current_state);
            m_usb_tx_string("\r\n");
            m_usb_tx_string("Puck (angle, see)");
            m_usb_tx_int((int) (100 * get_puck_angle()));
            m_usb_tx_string("\r\n");


            m_usb_tx_string("Pos (x, y, t): (");
            m_usb_tx_int((int) (getPosition()[0]));
            m_usb_tx_string(", ");
            m_usb_tx_int((int) (getPosition()[1]));
            m_usb_tx_string(", ");
            m_usb_tx_int((int) 100 * (getPosition()[2]));
            m_usb_tx_string(")\r\n");


            goalie();
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
    char address;
    if(ROBOT_NUMBER == 1) {
      address = ROBOT_1_ADDRESS;
    } else if(ROBOT_NUMBER == 2) {
      address = ROBOT_2_ADDRESS;
    } else {
      address = ROBOT_3_ADDRESS;
    }
    ROBOT_ADDRESSES[0] = ROBOT_1_ADDRESS;
    ROBOT_ADDRESSES[1] = ROBOT_2_ADDRESS;
    ROBOT_ADDRESSES[2] = ROBOT_3_ADDRESS;
    m_rf_open(CHANNEL, address, BUFFER_SIZE);

    //initialize USB
    if(USB_DEBUG || MATLAB_GRAPH || FORCE_USB) {
      m_usb_init();
    }

    //start timer0
    start0(TICK_FREQUENCY);
    interupt0(1);
    ADC_init();
    localize_init(TEAM_RED);
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

        if (TEAM_RED) {
            TEAM_RED = 0;
            clear(PORTB,3); // Change LED color at halftime
            set(PORTB,2);
        }
        else {
            TEAM_RED = 1;
            clear(PORTB,2);
            set(PORTB,3);
        }

    } else if(value == 0xA7) { // Game Over
      current_state = PAUSE;
    }
}

void handleRfCommand(char* rf_buffer) {
  m_usb_tx_string("Recieved Rf Command: ");
  if(((uint8_t)rf_buffer[2])) {
    m_usb_tx_string("Goalie\n");
  } else {
    m_usb_tx_string("Forward\n");
  }
  int i;
  m_usb_tx_string("Buffer:(");
  for(i = 0; i < 10; i++) {
    m_usb_tx_int((uint8_t)rf_buffer[i]);
    m_usb_tx_string("\t");
  }
  m_usb_tx_string(")\n\r");
}

void handleRfRobotInfo(char* rf_buffer) {
  m_usb_tx_string("Recieved Robot Info\n");
  int i;
  m_usb_tx_string("Buffer:(");
  for(i = 0; i < 10; i++) {
    m_usb_tx_int((uint8_t)rf_buffer[i]);
    m_usb_tx_string("\t");
  }
  m_usb_tx_string(")\n\r");
}

void handleRfPuckLocation(char* rf_buffer) {
  m_usb_tx_string("Recieved Puck Location\n");
  int i;
  m_usb_tx_string("Buffer:(");
  for(i = 0; i < 10; i++) {
    m_usb_tx_int((uint8_t)rf_buffer[i]);
    m_usb_tx_string("\t");
  }
  m_usb_tx_string(")\n\r");
}

int validateString(uint8_t rf_passcode) {
  if(rf_passcode == PASSCODE) {
    m_usb_tx_string("String Validation Succeded\n");
    return 1;
  } else {
    m_usb_tx_string("String Validation Failed\n");
    return 0;
  }
}

//m_rf flag setter
ISR(INT2_vect) {
    m_green(TOGGLE);
    RF_READ = 1;
    m_rf_read(buffer, BUFFER_SIZE);

    m_usb_tx_string("RF: ");
    m_usb_tx_hex(buffer[0]);
    m_usb_tx_string("\r\n");
    //handleRfGamestate((uint8_t) buffer[0]);
    uint8_t value = (uint8_t) buffer[0];
    uint8_t passcode = (uint8_t) buffer[1];
    if(value == 0xA8 && validateString(passcode)) { // Robot Game State Command
      handleRfCommand(buffer);
    } else if(value == 0xA9 && validateString(passcode)) { // Puck Location
      handleRfPuckLocation(buffer);
    } else if(value == 0xAA && validateString(passcode)) { // Robot Info
      handleRfRobotInfo(buffer);
    } else {
      handleRfGamestate(value);
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
            //m_usb_tx_string("TICK!\n\r");
        }
    }
}
