#ifndef SENSORFUNCTIONS_H
#define SENSORFUNCTIONS_H

#include "Plant_Drizzler.h"



void updateAllSensors();
void checkAmuxSensors();
void checkBmpSensors();

bool forceUpdateSensors();

int calcAverage(int arr[]);


#endif  /*SENSORFUNCTIONS_H*/