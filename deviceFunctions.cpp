#include "deviceFunctions.h"

// device states information
/* Possible states:
  0 --> 'wait': Bathroom isn't being used, default mode
  1 --> 'detection': Detecting use case
  2 --> '...': ...
*/
int deviceState = 0;
unsigned long deviceTimestamp = 0;


void deviceLoop(unsigned long curTime) {

}

bool deviceIsIdle() {
  return (deviceState == 0);
}

int temperature() {
  return 0;
}

void setSpraysShort(int newAmount) {
  //
}

void setSpraysLong(int newAmount) {
  //
}
