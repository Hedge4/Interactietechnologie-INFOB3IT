#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>


// return if timestampA is later in time than timestampB, taking rollover of millis() into account
// only use this if you don't know the interval between the timestamps, otherwise use function below
bool compareTimestamps(unsigned long timestampA, unsigned long timestampB);

// return if, after timestamp, interval has elapsed
bool compareTimestamps(unsigned long currentTime, unsigned long timestamp, unsigned long interval);


#endif /*FUNCTIONS_H*/
