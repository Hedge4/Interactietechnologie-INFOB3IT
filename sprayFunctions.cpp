#include "sprayFunctions.h"

/////////////////////////
//    CONFIGURATION    //
/////////////////////////
int sprayDuration = 25 * 1000; // 25 seconds
int pauseDuration = 5 * 1000; // 5 seconds

// current spray information
bool spraying = false;
bool waiting = false;
int spraysLeft = 0;
unsigned long sprayTimestamp = 0;
bool buttonPressed = false;


////////////////////////////
//          LOOP          //
////////////////////////////
void sprayLoop(unsigned long curTime) {
  if (spraying) {
    // stop spraying if we've reached sprayDuration
    if (compareTimestamps(curTime, sprayTimestamp + sprayDuration)) {
      spraying = false;
      waiting = true;
      // SPRAY DISABLE LOGIC HERE
      sprayTimestamp = millis();
      Serial.println("Spraying stopped, now waiting.");
    }
  } else if (waiting) {
    // stop waiting if we've reached pauseDuration
    if (compareTimestamps(curTime, sprayTimestamp + pauseDuration)) {
      waiting = false;
      if (spraysLeft > 0) {
        spraysLeft--;
        startSpray();
      } else {
        Serial.println("No more sprays, now idle.");
      }
    }
  }

  // if it was pressed, check if button no longer pressed
}


/////////////////////////////
//        FUNCTIONS        //
/////////////////////////////
void startSpray() {
  if (waiting || spraying) {
    // when still waiting or spraying, schedule another spray
    spraysLeft = (1 > spraysLeft) ? 1 : spraysLeft; // set to 1 if 0
    Serial.println("Remaining sprays set to: " + String(spraysLeft));
  } else {
    spraying = true;
    // TODO SPRAY ENABLE LOGIC HERE
    sprayTimestamp = millis();
    Serial.println("Spray started! Sprays left: " + String(spraysLeft));
  }
}

void startSpray(int amount) {
  amount--; // first spray is immediately activated so doesn't count
  spraysLeft = (amount > spraysLeft) ? amount : spraysLeft; // whichever value is higher
  startSpray();
}

// stop ongoing and cancel upcoming sprays
void cancelSprays() {
  spraysLeft = 0;
  if (spraying) {
    spraying = false;
    waiting = true;
    // TODO SPRAY DISABLE LOGIC HERE
    sprayTimestamp = millis();
    Serial.println("Spraying stopped, now waiting.");
  }
}

void sprayButtonUpdate(bool pressed) {
  // handle spray button change
  // we don't have to check current state, startSpray() does this
  if (pressed) {
    // only do something if button became pressed
    startSpray();
  }
}
