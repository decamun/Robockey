#define LOCALIZE_LPF 0.93

static uint16_t dropped_frames = 0;
static float LOCALIZE_CENTER_XY[2];
static float LOCALIZE_ANGLE;


void localize(int data[12]) {
  static int remove[4] = [0, 0, 0, 0];
  int i = 0;
  for(i = 0; i < 4; i++) {

    //check for unfound stars
    int removed = 0;
    if(data[i*3] == 1023 && data[(i+1)*3] == 1023) {
      remove[i] = 1;
      removed++;
    } else {
      remove[i] = 0;
    }


  }

  //do not continue if not enough stars found
  if(removed < 1) {
    dropped_frames = 0;

    //main function
    float distance [4,4];
    float max_distance = 0;
    float curr_distance = 0;
    int main_points[2];
    int other_points[2];
    int j = 0;

    //find max distance and main points
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) { //iterate through all point combinations (could probably be faster)
        if(i != j && !remove[i] && !remove[j]) { //don't do the ones that are zero or missing
          curr_distance = sqrtf(powf(data[i * 3] - data[j*3],2) + powf(data[(i+1)*3] - data[(j+i) * 3],2)); //pythagorean theorem to find the distance
          if (curr_distance > max_distance) {
            max_distance = curr_distance;
            main_points = [i, j];
          }
          distance[i, j] = curr_distance;
        }
      }
    }

    //build array of other points
    j = 0;
    for(i = 0; i < 4; i++) {
      if(main_points[1] != i && main_points[2] != i) {
        other_points[j] = i;
        j++;
      }
    }

    //find the north star
    float sum[2];
    for(i = 0; i < 2; i++) {
      sum[i] = distance[main_points[i], other_points[1]] + distance[main_points[i], other_points[2]];
    }
    int north_star = main_points[0];
    int south_star = main_points[1];
    if(sum[0] > sum[1]) {
      north_star = main_points[1];
      south_star = main_points[0];
    }

    //get angle and make transform
    float angle = atan2f(data[(north_star + 1) * 3] - data[(south_star + 1) * 3], data[(north_star) * 3] - data[(south_star) * 3]);
    float angle_adg  = angle + M_PI/2;
    float R[2, 2] = [cosf(-angle_adg), -sinf(-angle_adg); sinf(-angle_adg) cosf(-angle_adg)];
    float T[2] = [512, 384];
    float centerxy[2] = [(data[(north_star) * 3] + data[(south_star) * 3])/2 - T[1], (data[(north_star + 1) * 3] + data[(south_star + 1) * 3])/2 - T[2]];
    float centerxy_tx[2] = [-R[0, 1] * centerxy[0] - R[0, 1] * centerxy[1] , -[R[1, 0] * centerxy[0] - R[1, 1] * centerxy[1]];
    LOCALIZE_CENTER_XY = [LOCALIZE_CENTER_XY[0] * (LOCALIZE_LPF) + centerxy_tx[0] * (1 - LOCALIZE_LPF), LOCALIZE_CENTER_XY[1] * (LOCALIZE_LPF) + centerxy_tx[1] * (1 - LOCALIZE_LPF)]; //low pass transformed location
    LOCALIZE_ANGLE = -angle_adg; //save transformed angle of robot (negative because it is in field coordinates, not robot coordinates)

  } else {
    dropped_frames++;
  }
}


int* localize_location() {
  return &[LOCALIZE_CENTER_XY[0], LOCALIZE_CENTER_XY[1], LOCALIZE_ANGLE];
}
