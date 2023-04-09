#include "deviceFunctions.h"


/////////////////////////
//    CONFIGURATION    //
/////////////////////////

/* Possible states:
  0 --> 'idle': Controller isn't being used, default mode
  1 --> 'detect': Detecting gesture
*/
int deviceState = 0;
unsigned long deviceTimestamp = 0;


/////////////////////////
//      FUNCTIONS      //
/////////////////////////

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceTimestamp = millis();
    deviceState = newState;

    Serial.print(F("Switched to deviceState "));
    Serial.println(newState);

    // state initialisation logic
    switch (newState) {
      case 0:
        break;
      case 1:
        break;
    }
  }
}


//////////////////////////
//  EXTERNAL FUNCTIONS  //
//////////////////////////

// ! nothing here yet


/////////////////////////
//       GETTERS       //
/////////////////////////

// ! nothing here yet


////////////////////////////
//          LOOP          //
////////////////////////////

void deviceLoop(unsigned long curTime) {
  // ! nothing here yet

  switch (deviceState) {
    case 0:
      break;
    case 1:
      break;
  }
}
