#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>


// return if timestampA is later in time than timestampB, taking rollover of millis() into account
bool compareTimestamps(unsigned long timestampA, unsigned long timestampB);


#endif /*FUNCTIONS_H*/
