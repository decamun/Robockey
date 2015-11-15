#ifndef LOCALIZE
#define LOCALIZE

static uint16_t dropped_frames;
static float LOCALIZE_CENTER_XY[2];
static float LOCALIZE_ANGLE;
static char LOCALIZE_INIT;

void localize_init();
void localize_update();
void localize_calculate(int* data);
float* localize_location();


#endif
