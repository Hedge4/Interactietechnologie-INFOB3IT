#include "movementStorageClass.h"

/*
  This class is no longer used, but it would store the last 50 values from the MPU6050,
  and have separate detection methods for each gesture, which would be called by gestureDetection.cpp
  depending on the device state. I moved away from this for better performance and easier to understand
  code, but I'm leaving the class here as it would've supported more advanced detection algorithms.
*/

BlockNot storeBoundsInterval(1000); // how often the average upper/lower bounds are updated

MovementStorageClass::MovementStorageClass() {
  // no setup required
}

void MovementStorageClass::storeMpuValues(float roll, float pitch, float yaw, float accX, float accY, float accZ) {
  valueIterator = ++valueIterator < maxValuesSaved ? valueIterator : 0; // reset if we iterate past end of array

  rollValues[valueIterator] = roll;
  pitchValues[valueIterator] = pitch;
  yawValues[valueIterator] = yaw;
  accYValues[valueIterator] = accX;
  accYValues[valueIterator] = accY;
  accZValues[valueIterator] = accZ;
  
  // store bounds every few iterations
  if (storeBoundsInterval.triggered()) {
    if (valueIterator < nextBoundsUpdate) {
      // if true, valueIterator went past maxValuesSaved after the last update
      for (size_t i = nextBoundsUpdate; i < maxValuesSaved; i++) {
        // store bounds code
      }
      nextBoundsUpdate = 0; // set to 0 for the rest of the updated values
    }
    for (nextBoundsUpdate; nextBoundsUpdate <= valueIterator; nextBoundsUpdate++) {
      // store bounds code
    }
    // if we went past the end of the array, next iterator should be 0
    if (nextBoundsUpdate >= maxValuesSaved) nextBoundsUpdate = 0;
  }
}
