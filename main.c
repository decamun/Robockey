#include <avr/io.h>
#include <avr/wdt.h>
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
//#include "rf.h"

char buffer[BUFFER_SIZE];


//flags
int RF_READ = 0;
int TICK_HAPPENED = 0;
int TEAM_RED = 0;
int SIDE_RED = 0;
int SECOND_HALF = 0;
char ROBOT_ADDRESSES[3];
int ROBOT_INFO[3][7];
unsigned int TX_counter = 0;

int GO = 0;
int BLINK = 0;
int SEARCH_MODE = 0;
int ii = 0;

int KICK_TICKS = 0;
int LED_pin = 2;
int OFF_LED_Pin = 3;

int indicators[2] = {0,1};

float GUARD_X ;
float GUARD_Y ;
float MIDFIELD_SEARCH_X;
float MIDFIELD_SEARCH_Y;
int LAUNCH_timout = 0;

void report_error(const char *err);
void initialize();
void kick();

typedef enum {SEARCHING = 0, ACQUIRE, GOTO_GOAL, PUCK_TURN, PAUSE, PLAY, GOTO_ZERO,
              GOTO_GUARD, SEARCH_LEFT, SEARCH_RIGHT, TRACK, FACE_GUARD, LAUNCH} robot_state;
typedef enum {FORWARD, GOALIE} robot_role;

robot_state current_state = PLAY;
robot_role current_role = STARTING_ROLE;



void avoid_wall() {
  //TODO fix this
    /*while(localize_heading_for_wall() && localize_current()) {
      localize_update();
      update_puck_angle();
      if(get_puck_distance() > 50){
        setRight(-0.5f);
        setLeft(-0.5f);
      } else if(get_puck_angle() < 0) {
        setLeft(-0.5f);
        setRight(0.0f);
      } else {
        setRight(-0.5f);
        setLeft(0.0f);
      }

    }*/
}

void wait_for_play() {
  while(current_state == PAUSE) {
    m_wait(100);
  }
}

void test_kicker() {
  kick();
  m_wait(3000);
  wdt_reset();
}

void usb_debug(){
  m_usb_tx_string("State, Role: ");
  m_usb_tx_int(current_state);
  m_usb_tx_string(" , ");
  m_usb_tx_int(current_role);
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

  m_usb_tx_string("GUARD (X, Y): (");
  m_usb_tx_int(GUARD_X);
  m_usb_tx_string(", ");
  m_usb_tx_int(GUARD_Y);
  m_usb_tx_string(")\r\n");
}

void test_rf() {
  char out_buffer[10] = {0xAA,0x69,ROBOT_NUMBER,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
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
  wdt_reset();
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

void testMotors() { //TODO don't use this!
    setRight(0.85);
    m_wait(3000);
    setLeft(0.85);
    m_wait(3000);
    stop();
    m_wait(3000);
    setRight(-0.85);
    setLeft(-0.85);
    m_wait(3000);
    stop();
    m_wait(5000);
}

void fullTestMotor() { //TODO don't use this
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

void master() {
}

void slave() {

}

void goalie() {
    float angle = getPosition()[2];

    switch(current_state) {
        case PAUSE:
            clear(PORTD, LED_pin); // TURN OFF positioning LED
            stop();
            //wait_for_play();
            break;
        case PLAY:
            //current_state = GOTO_GUARD;
            if(get_see_puck()){
              current_state = TRACK;
            }
            break;
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
            if (fabs(getPosition()[2]) < 0.1f || fabs(getPosition()[2]) > 6.18f) {
                current_state = SEARCH_LEFT ;
            }
            break;
        case SEARCH_LEFT:
            setRight(-0.5f);
            setLeft(0.5f);
                if(angle > (M_PI_2 / 2.0f) && angle < M_PI) {
                    current_state = SEARCH_RIGHT;
                }

            if (get_see_puck()) {
                current_state = TRACK;
            }

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

            /*if (get_puck_position()[0] < -200 || get_puck_distance() < 10) {
                current_state = ACQUIRE;
            }*/
            if(get_puck_distance() < 60/*50*/) {
              current_state = LAUNCH;
              current_role = FORWARD;
            }
            break;
        case ACQUIRE:
            goToHeadingVel(0.8f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);

            if (!get_see_puck()) {
              current_state = SEARCHING;//GOTO_GUARD;
              current_role = FORWARD;
            }
            if (puck_middle()) {
              current_state = GOTO_GOAL;
              current_role = FORWARD;
              resetGoTo();
            }

            break;
        case GOTO_GOAL:
          goToPosition(getPosition(), 0.3f, 0.9f, GOAL_X, GOAL_Y);
          if(getPosition()[0] > 100 && fabs(getPosition()[1]) < 70 && negpi2pi(getPosition()[2]) < 1) {
            kick();
            stop();
            wdt_reset();
            m_wait(1000);
            current_state = SEARCHING;
          }

        if(!(puck_middle())) {
            current_state = ACQUIRE;
            resetGoTo();
        } else if (!get_see_puck()) {
            current_state = SEARCHING;
            resetGoTo();
        }

        break;
        default:
            current_state = PLAY;
            break;

    }
}

void forward() {
    switch(current_state) {
        case PAUSE:
            stop();
            //wait_for_play();
            break;

        case PLAY:
            current_state = LAUNCH;
            break;

        case LAUNCH:
            if(get_see_puck() && LAUNCH_timout < 100){
              if(get_puck_distance() < 50.0f){
                goToHeadingVel(0.95f + (get_puck_distance() - 50.0f)/200.0f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);
              } else {
                goToHeadingVel(0.95f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);
              }
            } else if(LAUNCH_timout < 25){
              setLeft(1.0f);
              setRight(1.0f);
              LAUNCH_timout++;
            } else {
              current_state = SEARCHING;
            }

            if(puck_middle()) {
              current_state = GOTO_GOAL;
            }

            if(puck_left() || puck_right()) {
              current_state = PUCK_TURN;
            }
            break;

        case SEARCHING:
            spin();
            avoid_wall();
            if(get_see_puck())  {
                current_state = ACQUIRE;
            }
            break;

        case ACQUIRE:

            if(get_puck_distance() < 45.0f) {
                float angle_factor = negpi2pi(getPosition()[2]);
                if(angle_factor > 3.14159f/2) {
                  angle_factor = 3.14159f - angle_factor;
                } else if(-angle_factor < -3.14159f/2) {
                  angle_factor = -3.14159f - angle_factor;
                }

                float offset_angle = (get_puck_angle() + angle_factor) * 0.2;
                m_usb_tx_string("Heading Offset: ");
                m_usb_tx_int((int16_t)(offset_angle * 100));
                m_usb_tx_string(" puck_angle: ");
                m_usb_tx_int((int16_t)(get_puck_angle() * 100));
                m_usb_tx_string(" robot_angle: ");
                m_usb_tx_int((int16_t)(negpi2pi(getPosition()[2]) * 100));
                m_usb_tx_string("\n\r");
                goToHeadingVel(0.5f, -get_puck_angle() - offset_angle, 0.0f, 1.2f, 0.7f);
            } else {
                goToHeadingVel(0.6f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);
            }

            if (puck_middle()) {
              current_state = GOTO_GOAL;
              resetGoTo();
            }
            if (puck_left() || puck_right()) {
              current_state = PUCK_TURN;
              resetGoTo();
            }

            if (!get_see_puck()) {
              current_state = SEARCHING;
              resetGoTo();
            }

            break;

        case GOTO_GOAL:
            goToPosition(getPosition(), 0.3f, 0.7f, GOAL_X, GOAL_Y);
            if(getPosition()[0] > 100 && fabs(getPosition()[1]) < 70 && negpi2pi(getPosition()[2]) < 1) {
              kick();
              stop();
              m_wait(1000);
              current_state = SEARCHING;
            }

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
                setRight(0.85f);
                setLeft(0.0f);
            } else if (puck_right()) {
                setLeft(0.85f);
                setRight(0.0f);
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
        default:
            current_state = PLAY;
            break;
    }
}

void midfield() {
    switch(current_state) {
        case PAUSE:
            clear(PORTD, LED_pin); // TURN OFF positioning LED
            stop();
            //wait_for_play();
            break;

        case PLAY:
            set(PORTD, LED_pin);
            current_state = SEARCHING;
            break;

        case SEARCHING:



          if(fabs(getPosition()[0] - MIDFIELD_SEARCH_X) < 50 && fabs(getPosition()[1] - MIDFIELD_SEARCH_Y) < 50) {
            spin();
          } else {
            goToPosition(getPosition(), 0.7f, 0.8f, MIDFIELD_SEARCH_X, MIDFIELD_SEARCH_Y);
          }
          avoid_wall();
          if(get_see_puck())  {
              current_state = ACQUIRE;
          }
          break;

        case ACQUIRE:

            if(get_puck_position()[0] < getPosition()[0]) {
              float position_offset_y = 0;
              if(fabs(getPosition()[1] - get_puck_position()[1]) > 50) {
                //do nothing
              } else if(getPosition()[1] > get_puck_position()[1]) {
                position_offset_y = -100;
              } else {
                position_offset_y = 100;
              }
              MIDFIELD_SEARCH_X = get_puck_position()[0] - 50;
              MIDFIELD_SEARCH_Y = getPosition()[1] + position_offset_y;
              goToPosition(getPosition(), 0.7f, 0.8f, MIDFIELD_SEARCH_X, MIDFIELD_SEARCH_Y);

            } else {
              goToHeadingVel(0.9f, -get_puck_angle(), 0.0f, 1.2f, 0.7f);
            }

            if (puck_middle()) {
              current_state = GOTO_GOAL;
              resetGoTo();
            }
            if (puck_left() || puck_right()) {
              current_state = PUCK_TURN;
              resetGoTo();
            }

            if (!get_see_puck()) {
              current_state = SEARCHING;
              resetGoTo();
            }

            break;

        case GOTO_GOAL:
            goToPosition(getPosition(), 0.3f, 0.7f, GOAL_X, GOAL_Y);
            if(getPosition()[0] > 100 && fabs(getPosition()[1]) < 70 && negpi2pi(getPosition()[2]) < 1) {
              kick();
              stop();
              m_wait(1000);
              current_state = SEARCHING;
            }

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
                setRight(0.85f);
                setLeft(0.0f);
            } else if (puck_right()) {
                setLeft(0.85f);
                setRight(0.0f);
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
        default:
            current_state = PLAY;
            break;
    }
}

void main()
{
  initialize();
  while (1) {
    wdt_reset();
    if(RF_READ) {
        //handle new RF info
      RF_READ = 0;

      //TODO figure out what rf_comm() is and if it should be there
      //rf_comm(buffer);

      uint8_t value = (uint8_t) buffer[0];
      uint8_t passcode = (uint8_t) buffer[1];
      handleRfGamestate(value);
      /*
      if(value == 0xA8 && validateString(passcode)) { // Robot Game State Command
        handleRfCommand(buffer);
      } else if(value == 0xA9 && validateString(passcode)) { // Puck Location
        handleRfPuckLocation(buffer);
      } else if(value == 0xAA && validateString(passcode)) { // Robot Info
        handleRfRobotInfo(buffer);
      } else {

      }
      */

      m_usb_tx_string("RF: ");
      m_usb_tx_hex(buffer[0]);
      m_usb_tx_string("\r\n");
    }

    if(TICK_HAPPENED && current_state != PAUSE) {
      // Get the current position and orientation
      usb_debug();
      localize_update();
      update_puck_angle();
      if (current_role == FORWARD) {
        forward();
      } else if (current_role == GOALIE) {
        goalie();
      } else {
        m_usb_tx_string("FATAL ERROR: INVALID ROLE\r\n");
      }


      if(KICK_TICKS > 0){KICK_TICKS--;} //awful spaghetti code TODO lol

      if(TX_counter > TX_INTERMISSION) {
        //sendRfRobotInfo();
        TX_counter = 0;
      } else {
        TX_counter++;
      }

      //doesnt work
      //update_indicators();

      // We're done until the next clock update
      TICK_HAPPENED = 0;


    } else if(current_state == PAUSE) {
      clear(PORTD, LED_pin); // TURN OFF positioning LED
      stop();
      resetGoTo();
    }
  }
}

void initialize() {
    m_clockdivide(0);

    //set watchdog timer
    wdt_reset();
    wdt_enable(WDTO_2S);

    //enable inputs for switches
    clear(DDRB, 4);
    set(PORTB, 4);
    clear(DDRB, 5);
    set(PORTB, 5);

    //enable outputs for green LEDs
    set(DDRD, 5);
    set(DDRD, 7);


    //Changing Output pin for different team
    if(puck_left()) {
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


    // Changing Output pin for different half
    if(puck_right()) {
      SECOND_HALF = 1;
    } else {
      SECOND_HALF = 0;
    }

    if(SECOND_HALF){
      set(PORTD, 5);
      set(PORTD, 7);
    }
    else {
      set(PORTD, 5);
      clear(PORTD, 7);
    }

    //determining the side (protect red or blue goal)

    if (TEAM_RED && !SECOND_HALF){
      SIDE_RED = 1;
    }
    else if (!TEAM_RED && SECOND_HALF){
      SIDE_RED = 1;
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
    set_indicators(INDICATE_STATES);

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

    m_usb_tx_string("RF Address: ");
    m_usb_tx_hex(address);
    m_usb_tx_string("\n\r");
    m_rf_open(CHANNEL, address, BUFFER_SIZE);

    //initialize USB
    if(USB_DEBUG || MATLAB_GRAPH || FORCE_USB) {
      m_usb_init();
    }

    //start timer0
    start0(TICK_FREQUENCY);
    interupt0(1);
    ADC_init();
    localize_init(SIDE_RED);

    //initialize state machine
    current_state = STARTING_STATE;
    current_role = STARTING_ROLE;

    MIDFIELD_SEARCH_X = MIDFIELD_START_X;
    MIDFIELD_SEARCH_Y = MIDFIELD_START_Y;

    //spaghetti code
    m_wait(30);
    int wait = 0;
    for(wait = 0; wait < 200; wait++) {localize_update();}
    GUARD_X = getPosition()[0];
    GUARD_Y = getPosition()[1];
    resetGoTo(); // Ensure that PD loops are set to 0
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
    if(KICK_TICKS ==0) {
      set(PORTB, 7);
      m_wait(200);
      clear(PORTB, 7);
      //KICK_TICKS = (int)(TICKS_PER_SECOND * 0.125);
    }
    KICK_TICKS = 200;
}

void distributePacket(char* out_buffer) {
  int i;
  for(i = 1; i <= 3; i++) {
    if(i != ROBOT_NUMBER) {
      m_usb_tx_string("Sending to Addess: ");
      m_usb_tx_int((uint8_t)ROBOT_ADDRESSES[i-1]);
      m_rf_send(ROBOT_ADDRESSES[i-1], out_buffer, BUFFER_SIZE);
      m_usb_tx_string("\n\r");
    }
  }
  m_usb_tx_string("Sending from Address: ");
  m_usb_tx_int((uint8_t)ROBOT_ADDRESSES[ROBOT_NUMBER-1]);
  m_usb_tx_string("\n\r");
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
        wdt_reset();
      }

    } else {
      for(ii = 0; ii<2; ii++){

        clear(PORTB,2);
        m_wait(200);
        set(PORTB,2);
        m_wait(200);
        wdt_reset();
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

    } else if(value == 0xA6) { // halftime
      current_state = PAUSE;

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
  int i;
  int j;
  m_usb_tx_string("Recieved Robot Info\n");
  m_usb_tx_string("Buffer:(");
  for(i = 0; i < 10; i++) {
    m_usb_tx_int((uint8_t)rf_buffer[i]);
    m_usb_tx_string("\t");
  }
  m_usb_tx_string(")\n\r");

  //transcode robot information to correct indeces
  for(i = 3; i < 10; i++) {
    ROBOT_INFO[-1 + (uint8_t)rf_buffer[2]][i-3] = (int8_t)rf_buffer[i];
  }

  m_usb_tx_string("Recieved new robot info. Info updated to: \n\r");
  for(i = 0; i < 3; i++) {
    for(j = 0; j < 7; j++) {
      m_usb_tx_int(ROBOT_INFO[i][j]);
      m_usb_tx_string("\t");
    }
    m_usb_tx_string("\n\r");
  }
}

void sendRfRobotInfo(){
  char rf_buffer[10];
  rf_buffer[0] = 0xAA;
  rf_buffer[1] = PASSCODE;
  rf_buffer[2] = (char)ROBOT_NUMBER;
  rf_buffer[3] = (char)current_state;
  rf_buffer[4] = 1; //TODO make this report position (GOALIE OR FORWARD)
  rf_buffer[5] = (char)localize_location()[0];
  rf_buffer[6] = (char)localize_location()[1];
  rf_buffer[7] = (char)localize_location()[2];
  rf_buffer[8] = 0;
  rf_buffer[9] = 0;
  rf_buffer[10] = 0;

  distributePacket(rf_buffer);
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

/*int validateString(uint8_t rf_passcode) {
  if(rf_passcode == PASSCODE) {
    m_usb_tx_string("String Validation Succeded\n");
    return 1;
  } else {
    m_usb_tx_string("String Validation Failed\n");
    return 0;
  }
}
*/

//m_rf flag setter
ISR(INT2_vect) {
    m_green(TOGGLE);
    cli();
    RF_READ = 1;
    m_rf_read(buffer, BUFFER_SIZE);


    //handleRfGamestate((uint8_t) buffer[0]);
    sei();
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












































//TODO Why not THANK YOU, MARK ZUCKERBERG, for your forward-thinking generosity! And congrats on becoming a dad! Mark Zuckerberg has announced that he is giving away $45 billion of Facebook stock. What you may not have heard is that he plans to give 10% of it away to people like YOU and ME! All you have to do is copy and paste this message into a post IMMEDIATELY. At midnight PST, Facebook will search through the day's posts and award 1000 people with $4.5 million EACH as a way of saying thank you for making Facebook such a powerful vehicle for connection and philanthropy. I hope someone I know gets a piece of the pie--let me know if you do!!!

//TODO What the fuck did you just fucking say about me, you little bitch? I’ll have you know I graduated top of my class in the Navy Seals, and I’ve been involved in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills. I am trained in gorilla warfare and I’m the top sniper in the entire US armed forces. You are nothing to me but just another target. I will wipe you the fuck out with precision the likes of which has never been seen before on this Earth, mark my fucking words. You think you can get away with saying that shit to me over the Internet? Think again, fucker. As we speak I am contacting my secret network of spies across the USA and your IP is being traced right now so you better prepare for the storm, maggot. The storm that wipes out the pathetic little thing you call your life. You’re fucking dead, kid. I can be anywhere, anytime, and I can kill you in over seven hundred ways, and that’s just with my bare hands. Not only am I extensively trained in unarmed combat, but I have access to the entire arsenal of the United States Marine Corps and I will use it to its full extent to wipe your miserable ass off the face of the continent, you little shit. If only you could have known what unholy retribution your little “clever” comment was about to bring down upon you, maybe you would have held your fucking tongue. But you couldn’t, you didn’t, and now you’re paying the price, you goddamn idiot. I will shit fury all over you and you will drown in it. You’re fucking dead, kiddo.
