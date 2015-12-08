#ifndef OPTIONS
#define OPTIONS

#define ROBOT_NUMBER 1
#define MASTER_NUMBER 2
#define ROBOT_1_ADDRESS 0x14
#define ROBOT_2_ADDRESS 0x15
#define ROBOT_3_ADDRESS 0x16
#define TX_INTERMISSION 30

#define DRIVE_PI 3.14159f
#define STARTING_ROLE GOALIE


#define FORCE_USB 1
#define USB_DEBUG 0
#define MATLAB_GRAPH 0
#define INDICATE_STATES GOTO_GOAL, SEARCHING


#define INITIAL_POWER 1

#define SHITTY 0

#define RF_DEBUG 0
#define RF_DEBUG_ADDRESS 0x16
#define ADDRESS 0x15
#define CHANNEL 1
#define PASSCODE 0x69

#define TICK_FREQUENCY 200
#define TICK_LENGTH 0.0128.0f
#define TICKS_PER_SECOND 78.125.0f

#define BUFFER_SIZE 10

#define MAX_INTENSITY_M_WII 2
#define LOCALIZE_LPF 0.93f

#define GOAL_X 330.0f
#define GOAL_Y 0.0f

//#define GUARD_X -270.0f
//#define GUARD_Y 0.0f

#define WALL_X 0.0f
#define WALL_Y 100.0f

#define BOUNDS_X 0//
#define BOUNDS_Y 0//
#define GOAL_WIDTH 0//

#define WALL_AVOIDANCE_PX 20

#endif
