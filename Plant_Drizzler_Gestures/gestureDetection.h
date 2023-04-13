#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "Plant_Drizzler_Gestures.h"
#include "mpuConnection.h"
// #include "movementStorageClass.h"

// methods
void gestureDetectionLoop();
void storeMpuValues(float roll, float pitch, float yaw, float accX, float accY, float accZ);


#endif /*GESTURE_DETECTION_H*/
