

#include <stdint.h>
#include "options.h"
#include <math.h>
#include "localize.h"
#include "globalVariables.h"
#include "m_wii.h"
#include "m_bus.h"
#include "m_usb.h"

static uint16_t dropped_frames = 0;
static float LOCALIZE_CENTER_XY[2] = {1111.0f, 1111.0f};
static float LOCALIZE_ANGLE = 1.5f;
static char LOCALIZE_INIT = 0;
static uint16_t data[12];
char rx_buffer;

static float location[3]; //for function localize_location
static float T[2] = {512.0f, 384.0f};

void localize_init() {  
  // Following code to get around possible init error m_wii_open
  char open = 0;
  open = m_wii_open();

  if(open) {
    LOCALIZE_INIT = 1;
  } else {
    report_error("Could not open the Wii camera");
  }

}

void localize_update() {
  if(USB_DEBUG && m_usb_isconnected()) {
    m_usb_tx_string("Started to Localize\n\r");
  }

  if(!LOCALIZE_INIT) {
    localize_init();
  }

  if(m_wii_read(data)) {
    //USB DEBUG CODE
    if((USB_DEBUG || MATLAB_GRAPH )&& m_usb_isconnected()) {
      if(USB_DEBUG){
        m_usb_tx_string("M_Wii_Values:\n\r");
      } else {
        m_red(ON);
        //while(!m_usb_rx_available());
        rx_buffer = m_usb_rx_char();
        m_red(OFF);
      }
      if(!MATLAB_GRAPH || rx_buffer || 1) {
        //m_usb_rx_flush();
        m_red(ON);
        int i;
        for(i = 0; i < 4; i++) {
          //m_usb_tx_int(data[i*3]);
          //m_usb_tx_string("\t");
          //m_usb_tx_int(data[i*3 + 1]);
          //m_usb_tx_string("\t");
        }
        m_usb_tx_string("\n\r");
      }
    }
    //RF debug CODE
    //************UNFINISHED**********
    //if(RF_DEBUG) {
    //  char DEBUFFER[16];
    //  int i;
    //  for(i = 0, i < 4, i++) {
    //    *DEBUFFER[i * 2] = &data[i * 3];
    //    *DEBUFFER[i * 2 + 1] = &data[i * 3 + 1];
    //  }
    //}
    //***********\UNFINISHED**********




    //calculate position
    localize_calculate(data);

    //set global position
    setPosition(localize_location());

    //debug
    if(USB_DEBUG && m_usb_isconnected()) {
      m_usb_tx_string("Finished Localizing. Dropped Frames: ");
      m_usb_tx_int(dropped_frames);
      m_usb_tx_string("\n\r");
    }
  } else {
     report_error("Could not connect to USB in localize");
  }
}

void localize_calculate(uint16_t* data)
{
  static int remove[4] = {0, 0, 0, 0};
  int i = 0;
  int removed = 0;
  for(i = 0; i < 4; i++) {
    //check for unfound stars

    if((data[i*3] == 1023 && data[(i)*3 + 1] == 1023) || data[(i*3) + 2] > MAX_INTENSITY_M_WII) {
      remove[i] = 1;
      removed++;
      if(USB_DEBUG && m_usb_isconnected()) {
        m_usb_tx_string("removed point: X:");
        m_usb_tx_int((int)data[i*3]);
        m_usb_tx_string(" Y:");
        m_usb_tx_int((int)data[i*3 + 1]);
        m_usb_tx_string(" i:");
        m_usb_tx_int(i);
        m_usb_tx_string("\n\r");
      }
    }
	else {
      remove[i] = 0;
    }
  }

  //do not continue if not enough stars found
  if(removed < 2) {
    dropped_frames = 0;

    //main function
    float distance [4][4];
    float max_distance = 0.0f;
    float curr_distance = 0.0f;
    int main_points[2];
    int other_points[2];
    int j = 0;

    //find max distance and main points
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) { //iterate through all point combinations (could probably be faster)
        if(i != j && !remove[i] && !remove[j]) { //don't do the ones that are zero or missing
          float delta_x = abs((float)data[i*3] - (float)data[j*3]);
          float delta_y = abs((float)data[i*3 + 1] - (float)data[j*3 + 1]);
          curr_distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

          if (curr_distance > max_distance) {
            max_distance = curr_distance;
            main_points[0] = i;
			      main_points[1] = j;
          }
          distance[i][j] = curr_distance;
        }
      }
    }

    //build array of other points
    j = 0;
    for(i = 0; i < 4; i++) {
      if(main_points[0] != i && main_points[1] != i) {
        other_points[j] = i;
        j++;
      }
    }

    //find the north star
    float sum[2];
    for(i = 0; i < 2; i++) {
      sum[i] = distance[main_points[i]][other_points[0]] + distance[main_points[i]][other_points[1]];
    }
    int north_star = main_points[0];
    int south_star = main_points[1];
    if(sum[0] > sum[1]) {
      north_star = main_points[1];
      south_star = main_points[0];
    }

    //get angle and make transform
    float angle = atan2f(-(float)data[(north_star) * 3 + 1] + (float)data[(south_star) * 3 + 1], (float)data[(north_star) * 3] - (float)data[(south_star) * 3]);
    float angle_adg  = angle;// - M_PI/2;

    float centerxy[2];
    centerxy[0] = (float)(data[(north_star) * 3] + data[(south_star) * 3])/2 -T[0];
    centerxy[1] = (float)(data[(north_star) * 3 + 1] + data[(south_star ) * 3 + 1])/2 -T[1];

    float centerxy_tx[2];
    centerxy_tx[1] = -cosf(angle_adg) * centerxy[0] + sinf(angle_adg) * centerxy[1];
    centerxy_tx[0] = -sinf(angle_adg) * centerxy[0] - cosf(angle_adg) * centerxy[1];

    LOCALIZE_CENTER_XY[0] = LOCALIZE_CENTER_XY[0] * (LOCALIZE_LPF) + centerxy_tx[0] * (1 - LOCALIZE_LPF);
	  LOCALIZE_CENTER_XY[1] = LOCALIZE_CENTER_XY[1] * (LOCALIZE_LPF) + centerxy_tx[1] * (1 - LOCALIZE_LPF); //low pass transformed location

    if(angle_adg > 0) {
      angle_adg = angle_adg - 2.0f * DRIVE_PI;
    }
    LOCALIZE_ANGLE = LOCALIZE_ANGLE * (LOCALIZE_LPF) + (-angle_adg) * (1.0f - LOCALIZE_LPF);


    //USB DEBUG CODE
    if(MATLAB_GRAPH && m_usb_isconnected()) {
      while(!m_usb_rx_available());
      m_usb_rx_flush();
      m_red(ON);
      m_usb_tx_int((int)(centerxy_tx[0]));
      m_usb_tx_string("\t");
      m_usb_tx_int((int)(centerxy_tx[1]));
      m_usb_tx_string("\t");
      m_usb_tx_int((int)(-100.0f*(angle_adg+ DRIVE_PI)));
      m_usb_tx_string("\n");
    }


    if(USB_DEBUG && m_usb_isconnected()) {
      m_usb_tx_string("\n\n\rValues in funcion \n\r");
      m_usb_tx_string("X: ");
      m_usb_tx_int((int)(centerxy_tx[0]));
      m_usb_tx_string("\n\rY: ");
      m_usb_tx_int((int)(centerxy_tx[1]));
      m_usb_tx_string("\n\r0_X: ");
      m_usb_tx_int((int)(centerxy[0]));
      m_usb_tx_string("\n\r0_Y: ");
      m_usb_tx_int((int)(centerxy[1]));
      m_usb_tx_string("\n\rTheta: ");
      m_usb_tx_int((int)(100*angle_adg));

      m_usb_tx_string("\n\rPointer Address in localize: ");
      m_usb_tx_int(location);


      m_usb_tx_string("\n\rNorth Star: X:");
      m_usb_tx_int(data[north_star * 3]);
      m_usb_tx_string(" Y:");
      m_usb_tx_int(data[(north_star) * 3 + 1]);
      m_usb_tx_string(" i:");
      m_usb_tx_int(north_star);

      m_usb_tx_string("\n\rSouth Star: X:");
      m_usb_tx_int(data[south_star * 3]);
      m_usb_tx_string(" Y:");
      m_usb_tx_int(data[(south_star) * 3 + 1]);
      m_usb_tx_string(" i:");
      m_usb_tx_int(south_star);
      m_usb_tx_string("\n\r");


    }

  } else {
    dropped_frames++;
  }
}


float* localize_location() {
  location[0] = LOCALIZE_CENTER_XY[0];
  location[1] = LOCALIZE_CENTER_XY[1];
  location[2] = LOCALIZE_ANGLE;
  return location;
}
