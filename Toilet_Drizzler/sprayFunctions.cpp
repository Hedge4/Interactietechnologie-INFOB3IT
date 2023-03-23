#include "sprayFunctions.h"

/////////////////////////
//    CONFIGURATION    //
/////////////////////////

int sprayDuration = 25 * 1000; // 25 seconds
int pauseDuration = 5 * 1000; // 5 seconds
unsigned long delayBeforeSpray = 0;
unsigned long sprayDelayStartTime;

// current spray information
bool spraying = false;
bool waiting = false;
int plannedSpraysLeft = 0;
unsigned long sprayTimestamp = 0;
// bool buttonPressed = false;


////////////////////////////
//          LOOP          //
////////////////////////////

void sprayLoop(unsigned long curTime) {
  // if menu is active, interrupt sprays and save them for later
  if (menuActive()) {
    if (spraying) {
      plannedSpraysLeft++;
      spraying = false;
      waiting = true;
      digitalWrite(sprayPin, LOW);
      sprayTimestamp = millis();
      yellowLed = 0;
    }
    // only execute the code if menu isn't active
    return;
  }

  if (spraying) {
    // stop spraying if we've reached sprayDuration
    if (compareTimestamps(curTime, sprayTimestamp, sprayDuration)) {
      spraying = false;
      waiting = true;
      digitalWrite(sprayPin, LOW);
      sprayTimestamp = millis();
      Serial.println("Spraying stopped, now waiting.");

      // if there's another spray coming, we indicate that with a yellow led
      if (plannedSpraysLeft > 0) yellowLed = 1;
      // if there's less than 5% of sprays left in the device, we show a constantly burning yellow led
      else if ((spraysLeft * 100l) / defaultTotalSprays < 5) yellowLed = 1;
      // and if neither, turn the led off
      else yellowLed = 0;
    }
  } else if (waiting) {
    // stop waiting if we've reached pauseDuration
    if (compareTimestamps(curTime, sprayTimestamp, pauseDuration)) {
      waiting = false;
      // if there are more sprays left and there's no delay set, start spraying again
      if (plannedSpraysLeft > 0 && delayBeforeSpray == 0) {
        if (spraysLeft > 0) plannedSpraysLeft--;
        startSpray();
      } else {
        Serial.println("No more sprays, now idle.");
      }
    }
  } else {
    // check if there is a delay before we need to start spraying again and if it's elapsed
    if (delayBeforeSpray > 0) {
      // if no sprays left there's no point in the delay
      if (spraysLeft <= 0) {
        delayBeforeSpray = 0;
        return;
      }

      // check if delay has elapsed, and if so start a spray
      if (compareTimestamps(curTime, sprayDelayStartTime, delayBeforeSpray)) {
        delayBeforeSpray = 0;
        plannedSpraysLeft--;
        startSpray();
      }
    }
  }
}


/////////////////////////////
//        FUNCTIONS        //
/////////////////////////////

void startSpray() {
  if (waiting || spraying) {
    // when still waiting or spraying, schedule another spray
    plannedSpraysLeft = (1 > plannedSpraysLeft) ? 1 : plannedSpraysLeft; // set to 1 if 0
    Serial.print(F("Remaining sprays set to: "));
    Serial.println(plannedSpraysLeft);
  } else {
    spraying = true;
    yellowLed = 3; // the yellow led indicates the device is active with a fast blinking led
    if (spraysLeft > 0) spraysLeft--;
    // TODO EEPROM UPDATE LOGIC HERE
    digitalWrite(sprayPin, HIGH);
    sprayTimestamp = millis();
    Serial.print(F("Spray started! Sprays left: "));
    Serial.println(plannedSpraysLeft);
  }
}

void startSpray(int amount) {
  amount--; // first spray is immediately activated so doesn't count
  plannedSpraysLeft = (amount > plannedSpraysLeft) ? amount : plannedSpraysLeft; // whichever value is higher
  startSpray();
}

void startSpray(int amount, long waitUntilSpray) {
  waitUntilSpray-= 15 * 1000; // 15 seconds wait period is built in
  plannedSpraysLeft = (amount > plannedSpraysLeft) ? amount : plannedSpraysLeft; // whichever value is higher

  // if value was 15000 (and now 0), start immediately
  if (waitUntilSpray <= 0) {
    plannedSpraysLeft--; // first spray is immediately activated so doesn't count
    startSpray();
    return;
  }

  // we need to wait, so cancel current sprays
  cancelSprays();

  yellowLed = 1; // signify there are more sprays coming
  delayBeforeSpray = waitUntilSpray; // after this delay, will switch to spraying
  sprayDelayStartTime = millis();
  Serial.print(F("Waiting before spraying (ms): "));
  Serial.println(waitUntilSpray);
}

// stop ongoing and cancel upcoming sprays
void cancelSprays() {
  plannedSpraysLeft = 0;
  if (spraying) {
    spraying = false;
    waiting = true;
    digitalWrite(sprayPin, LOW);
    sprayTimestamp = millis();
    Serial.println("Spraying stopped, now waiting.");
  }
}

void sprayButtonUpdate(bool pressed) {
  // handle spray button change
  // we don't have to check current state, startSpray() does this
  if (!pressed) {
    // only do something if button was released
    startSpray();
  }
}


/////////////////////////////
//         GETTERS         //
/////////////////////////////

// whether a spray is ongoing or coming
bool sprayComing() {
  return (spraying || plannedSpraysLeft > 0);
}
