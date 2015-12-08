#ifndef LOCALIZE
#define LOCALIZE

static uint16_t dropped_frames;
static float LOCALIZE_CENTER_XY[2];
static float LOCALIZE_ANGLE;
static char LOCALIZE_INIT;

void localize_init(int TEAM_RED);
void localize_update();
void localize_calculate(uint16_t* data);
float* localize_location();
int localize_heading_for_wall();
int localize_current();


#endif
