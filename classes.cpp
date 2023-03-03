#include "classes.h"

// BUTTON CLASS
// constructor for button class
Knop::Knop(int l, int h) {
  lowVolt = l;
  highVolt = h;
}

// update function for buttons
void Knop::update(int volt, unsigned long curTime) {
  //reading is HIGH if button is pressed, LOW if not
  //get voltage from analog pin A5
  int reading = LOW;
  if (volt >= lowVolt && volt <= highVolt) {
    reading = HIGH;
  }

  // reset debounceTime if buttonstate changed
  if (reading != lastButtonState) {
    lastDebounceTime = curTime;
  }

  //compare last time button is pressed, skip if too fast (this is debounce part)
  if (compareTimestamps(curTime, lastDebounceTime, debounceDelay)) {
    //if the buttonstate has changed, update buttonState
    if (reading != buttonState) {
      buttonState = reading;
      changed = true;
      // if the buttonstate has changed to HIGH (button is pressed), update status
      if (buttonState == HIGH) {
        pressed = true;
      } else {
        pressed = false;
      }
    } else {
      changed = false;
    }
  }

  // save the reading. Next update call, it'll be the lastButtonState:
  lastButtonState = reading;
}
