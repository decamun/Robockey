#ifndef LOCALIZE
#define LOCALIZE

static uint16_t dropped_frames;
static float LOCALIZE_CENTER_XY[2];
static float LOCALIZE_ANGLE;

void localize(int data[12]);
int* localize_location();


#endif