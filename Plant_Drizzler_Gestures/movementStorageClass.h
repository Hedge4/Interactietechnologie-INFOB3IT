#ifndef CLASSES_H
#define CLASSES_H

#include <BlockNot.h>

const int maxValuesSaved = 50;      // how many values of each type should be saved before being overwritten

// button class
class MovementStorageClass {
  bool firstBoundsUpdate = true;    // if this is the first bounds update (so we don't go back past 0)
  int nextBoundsUpdate;             // specifies how far along the value array bounds were last updated
  int boundsValueIterator;  
  int valueIterator;                // specifies at what point in the value array the next value should be saved (goes back to 0 at maxValuesSaved)
  float rollValues[maxValuesSaved];
  float pitchValues[maxValuesSaved];
  float yawValues[maxValuesSaved];
  float accXValues[maxValuesSaved];
  float accYValues[maxValuesSaved];
  float accZValues[maxValuesSaved];
  // float rollBounds[2];
  // float pitchBounds[2];
  // float yawBounds[2];
  // float accXBounds[2];
  // float accYBounds[2];
  // float accZBounds[2];

public:
  MovementStorageClass();
  void storeMpuValues(float roll, float pitch, float yaw, float accX, float accY, float accZ);
};

#endif /*CLASSES_H*/
