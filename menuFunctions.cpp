#include "menuFunctions.h"

/////////////////////////
//    CONFIGURATION    //
/////////////////////////

/* Possible states:
  0 --> 'off': Nothing is displayed (empty bathroom)
  1 --> 'display': Display default info
  2 --> 'menu': User is interacting with the menu
  3 --> 'config': User is changing one of the settings
*/
int menuState = 0;
unsigned long menuTimestamp = 0;                  // when menuState last changed

String topText, bottomText;             // top and bottom row of lcd screen
int scrollIndex;                        // stores how far text has scrolled to the left
unsigned long scrollTimestamp;          // when scroll position was last updated
const int positionsPerScroll = 3;       // how many positions the text scrolls with each position update
const int spaceBetweenScrolls = 3;      // how many spaces to add before text repeats
const int scrollShortDelay = 450;       // how long before scroll position updates
const int scrollLongDelay = 3000;       // time to pause at initial position before scrolling again
unsigned long displayRefreshTimestamp;  // when info on screen in display state was last updated
const int displayRefreshDelay = 1000;   // refresh delay for info in display state

/* Possible settings:
   0 --> 'short': How many sprays for a short toilet visit
   1 --> 'long': How many sprays for a long toilet visit
*/
int currentSetting, currentValue;
const int maxConfig = 5, minConfig = 0;     // bounds for all settings
const int configDelay = 30 * 1000;          // 30 seconds, then leaves menu or config state
int sprayConfigList[2] = { 1, 2 };          // default settings
const String sprayConfigDesc[2] = { "Sprays na kort bezoek", "Sprays na lang bezoek" };


/////////////////////////
//      FUNCTIONS      //
/////////////////////////

String displayStateTopText() {
  return "Temperature: " + String(temperature()) + "   Sprays left: " + String(spraysLeft);
}

String displayStateBottomText() {
  String text = "State: " + deviceStateString();
  if (sprayComing()) text = text + "+S"; //

  return text;
}

void powerBacklight(bool power) {
  if (power) {
    digitalWrite(lcdBacklightPin, HIGH);
  } else {
    digitalWrite(lcdBacklightPin, LOW);
  }
}

// bottom text has no scrolling, and this function is called when it changes
void updateBottomText() {
  lcd.setCursor(0, 1);
  lcd.print(bottomText);
}

// prints text at one position based on scrollIndex
void updateTextScroll() {
  lcd.setCursor(0, 0);
  int x = scrollIndex;
  for (int i = 0; i < 16; i++) {
    lcd.print(topText[x]);
    x++;
    if (x >= topText.length()) x = 0;
  }
}

// updates scrollIndex whenever the delay expires
void scrollTextLoop(unsigned long curTime) {
  // only scroll if the text doesn't fit
  if (topText.length() > 16 + spaceBetweenScrolls) { // +x because we added x whitespace
    // pause a while longer if front of text is currently displayed
    if (scrollIndex == 0) {
      if (compareTimestamps(curTime, scrollTimestamp, scrollLongDelay)) {
        scrollIndex = scrollIndex + positionsPerScroll;
        scrollTimestamp = millis();
        updateTextScroll();
      }
    } else {
      if (compareTimestamps(curTime, scrollTimestamp, scrollShortDelay)) {
        scrollIndex = scrollIndex + positionsPerScroll;
        if (scrollIndex >= topText.length()) scrollIndex = 0;
        scrollTimestamp = millis();
        updateTextScroll();
      }
    }
  }
}

// resetScrollPos can be set to false if the new text is very similar to the old text
void setText(bool resetScrollPos = true) {
  // new text was set, so we do this again: add x whitespace at end of scrolling text
  if (topText.length() > 16) {
    for (int i = 0; i < spaceBetweenScrolls; i++) {
      topText = topText + " ";
    }
  }

  // if this isn't reset text will be printed at roughly the same position (if character count is similar enough)
  if (resetScrollPos) {
    scrollTimestamp = millis();
    scrollIndex = 0;
  }

  lcd.clear();
  if (scrollIndex != 0) scrollTimestamp = millis();
  updateTextScroll(); // if text isn't large enough to scroll it will behave like .print()
  updateBottomText();
}

// setup for when we enter a specific state
void changeMenuState(int newState) {
  if (menuState != newState) {
    menuState = newState;
    menuTimestamp = millis();

    // state initialisation logic
    switch (newState) {
      case 0:
        powerBacklight(false);
        topText = "";
        bottomText = "";
        lcd.clear();
        break;
      case 1:
        powerBacklight(true);
        topText = displayStateTopText();
        bottomText = displayStateBottomText();
        lcd.clear();
        setText();
        break;
      case 2:
        currentSetting = 0;
        currentValue = sprayConfigList[currentSetting];
        topText = sprayConfigDesc[currentSetting];
        bottomText = "Huidig: " + String(currentValue);
        setText();
        break;
      case 3:
        bottomText = "Nieuw: " + String(currentValue);
        setText();
        break;
    }
  }
}


////////////////////////
// EXTERNAL FUNCTIONS //
////////////////////////

// what happens if the menu button is pressed, based on state
void menuButtonUpdate(bool pressed) {
  if (pressed) {
    switch (menuState) {
      case 0:
        changeMenuState(1);
        break;
      case 1:
        changeMenuState(2);
        break;
      case 2:
        // change our current setting
        switch (currentSetting) {
          case 0:
            currentSetting = 1;
            break;
          case 1:
            currentSetting = 0;
            break;
        }
        // update values for new setting
        currentValue = sprayConfigList[currentSetting];
        topText = sprayConfigDesc[currentSetting];
        bottomText = "Huidig: " + String(currentValue);
        setText();
        break;
      case 3:
        if (currentValue < maxConfig) {
          currentValue++;
        } else {
          currentValue = minConfig;
        }
        bottomText = "Nieuw: " + String(currentValue);
        updateBottomText();
        break;
    }
  }
}

// what happens if the ok button is pressed, based on state
void okButtonUpdate(bool pressed) {
  if (pressed) {
    switch (menuState) {
      case 0:
        changeMenuState(1);
        break;
      case 1:
        changeMenuState(2);
        break;
      case 2:
        changeMenuState(3);
        break;
      case 3:
        // update setting locally
        sprayConfigList[currentSetting] = currentValue;
        // update setting in device controller
        if (currentSetting == 0) {
          setSpraysShort(currentValue);
        } else if (currentSetting == 1) {
          setSpraysLong(currentValue);
        }
        // finally change state back to display mode
        changeMenuState(1);
        break;
    }
  }
}

// used to turn the screen on if someone enters the room
void activateScreen() {
  // only does something if screen was turned off
  if (menuState == 0) {
    changeMenuState(1);
  }
}

// whether the system is in the menu or config state
bool menuActive() {
  return (menuState == 2 || menuState == 3);
}


////////////////////////////
//          LOOP          //
////////////////////////////

// the menu update loop called each tick
void menuLoop(unsigned long curTime) {
  // handles scrolling of text if necessary
  scrollTextLoop(curTime);

  if (menuState == 1) {
    // turn display off if bathroom is unused AND display is inactive for too long
    if (compareTimestamps(curTime, menuTimestamp, configDelay)) {
      if (deviceIsIdle()) {
        changeMenuState(0);
      }
    }

    // if info on the display changed, update text
    if (compareTimestamps(curTime, displayRefreshTimestamp, displayRefreshDelay)) {
      displayRefreshTimestamp = curTime;
      String newBottomText = displayStateBottomText();
      String newTopText = displayStateTopText();

      if (newTopText != topText) {
        // setText() will update both top and bottom text, so set both
        topText = newTopText;
        bottomText = newBottomText;
        setText(false); // false means we don't update the scroll position
      } else if (newTopText != topText) {
        // don't interfere with topText if we don't have to, because of scrolling
        bottomText = newBottomText;
        updateBottomText();
      }
    }
  }

  // from settings or config state, go back to display state if inactive for too long
  if (menuState == 2 || menuState == 3) {
    if (compareTimestamps(curTime, menuTimestamp, configDelay)) {
      changeMenuState(1);
    }
  }
}
