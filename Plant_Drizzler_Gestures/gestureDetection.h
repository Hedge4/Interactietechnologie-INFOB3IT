#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "Plant_Drizzler_Gestures.h"
#include "mpuConnection.h"

// methods
void gestureDetectionLoop();
void storeMpuValues(float roll, float pitch, float yaw, float correctedAccX, float correctedAccY, float correctedAccZ);


#endif /*GESTURE_DETECTION_H*/
