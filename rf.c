//RF COMM file
#include "m_general.h"
#include "m_bus.h"
#include "m_usb.h"


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

int validateString(uint8_t rf_passcode) {
  if(rf_passcode == PASSCODE) {
    m_usb_tx_string("String Validation Succeded\n");
    return 1;
  } else {
    m_usb_tx_string("String Validation Failed\n");
    return 0;
  }
}
